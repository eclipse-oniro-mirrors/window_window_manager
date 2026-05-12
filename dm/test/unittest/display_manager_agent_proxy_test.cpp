/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include <iremote_broker.h>
#include <iremote_object_mocker.h>
#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "display_manager.h"
#include "display_manager_adapter.h"
#include "display_manager_proxy.h"
#include "scene_board_judgement.h"
#include "window_scene.h"
#include "zidl/display_manager_agent_proxy.h"
#include "test/mock/mock_message_parcel.h"

using namespace testing;
using namespace testing::ext;

namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_logMsg = msg;
    }
}

namespace OHOS::Rosen {
class DisplayManagerAgentProxyTest : public testing::Test {
public:
    static void SetUpTestSuite();
    void SetUp() override;
    void TearDown() override;
    sptr<DisplayManagerAgentProxy> displayManagerAgentProxy;
};

void DisplayManagerAgentProxyTest::SetUpTestSuite()
{
}

void DisplayManagerAgentProxyTest::SetUp()
{
    sptr<IRemoteObject> impl;
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        impl = sptr<IRemoteObjectMocker>::MakeSptr();
    } else {
        impl = sptr<IRemoteObjectMocker>::MakeSptr();
    }

    displayManagerAgentProxy = sptr<DisplayManagerAgentProxy>::MakeSptr(impl);
    ASSERT_NE(displayManagerAgentProxy, nullptr);
}

void DisplayManagerAgentProxyTest::TearDown()
{
    displayManagerAgentProxy = nullptr;
}

/**
 * @tc.name: NotifyDisplayPowerEvent02
 * @tc.desc: NotifyDisplayPowerEvent
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyDisplayPowerEvent02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    DisplayPowerEvent event = DisplayPowerEvent::DESKTOP_READY;
    EventStatus status = EventStatus::BEGIN;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->NotifyDisplayPowerEvent(event, status);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyDisplayPowerEvent03
 * @tc.desc: NotifyDisplayPowerEvent
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyDisplayPowerEvent03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    DisplayPowerEvent event = DisplayPowerEvent::DESKTOP_READY;
    EventStatus status = EventStatus::BEGIN;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    displayManagerAgentProxy->NotifyDisplayPowerEvent(event, status);
    EXPECT_TRUE(g_logMsg.find("Write event failed") != std::string::npos);
}

/**
 * @tc.name: NotifyDisplayStateChanged01
 * @tc.desc: NotifyDisplayStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyDisplayStateChanged01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    DisplayId displayId = 0;
    DisplayState state = DisplayState::OFF;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->NotifyDisplayStateChanged(displayId, state);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyDisplayStateChanged02
 * @tc.desc: NotifyDisplayStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyDisplayStateChanged02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    DisplayId displayId = 0;
    DisplayState state = DisplayState::OFF;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    displayManagerAgentProxy->NotifyDisplayStateChanged(displayId, state);
    EXPECT_TRUE(g_logMsg.find("Write DisplayState failed") != std::string::npos);
}

/**
 * @tc.name: NotifyCaptureStatusChanged01
 * @tc.desc: NotifyCaptureStatusChanged
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyCaptureStatusChanged01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->NotifyCaptureStatusChanged(false);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyCaptureStatusChanged02
 * @tc.desc: NotifyCaptureStatusChanged
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyCaptureStatusChanged02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    displayManagerAgentProxy->NotifyCaptureStatusChanged(false);
    EXPECT_TRUE(g_logMsg.find("Write isCapture failed") != std::string::npos);
}

/**
 * @tc.name: NotifyDisplayChangeInfoChanged
 * @tc.desc: NotifyDisplayChangeInfoChanged
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyDisplayChangeInfoChanged, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<DisplayChangeInfo> display_change_info = sptr<DisplayChangeInfo>::MakeSptr();

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->NotifyDisplayChangeInfoChanged(display_change_info);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyDisplayChangeInfoChanged
 * @tc.desc: NotifyDisplayChangeInfoChanged
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyBrightnessInfoChanged, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenBrightnessInfo info;
    auto proxy = sptr<DisplayManagerAgentProxy>::MakeSptr(nullptr);

    proxy->NotifyBrightnessInfoChanged(0, info);
    EXPECT_TRUE(g_logMsg.find("remote is nullptr") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->NotifyBrightnessInfoChanged(0, info);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    displayManagerAgentProxy->NotifyBrightnessInfoChanged(0, info);
    EXPECT_TRUE(g_logMsg.find("Write screenId failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteFloatErrorFlag(true);
    displayManagerAgentProxy->NotifyBrightnessInfoChanged(0, info);
    EXPECT_TRUE(g_logMsg.find("write info failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    g_logMsg.clear();
    displayManagerAgentProxy->NotifyBrightnessInfoChanged(0, info);
    EXPECT_TRUE(g_logMsg.find("write info failed") != std::string::npos);
    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: NotifyDisplayModeChanged01
 * @tc.desc: NotifyDisplayModeChanged
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyDisplayModeChanged01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    FoldDisplayMode mode = FoldDisplayMode::UNKNOWN;
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->NotifyDisplayModeChanged(mode);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyDisplayModeChanged02
 * @tc.desc: NotifyDisplayModeChanged
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyDisplayModeChanged02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    FoldDisplayMode mode = FoldDisplayMode::UNKNOWN;
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    displayManagerAgentProxy->NotifyDisplayModeChanged(mode);
    EXPECT_TRUE(g_logMsg.find("Write displayMode failed") != std::string::npos);
}
/**
 * @tc.name: NotifyAvailableAreaChanged
 * @tc.desc: NotifyAvailableAreaChanged
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyAvailableAreaChanged, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    DisplayId displayId = 0;
    DMRect rect = {2, 2, 2, 2};

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->NotifyAvailableAreaChanged(rect, displayId);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}


/**
 * @tc.name: NotifyAvailableAreaChanged02
 * @tc.desc: NotifyAvailableAreaChanged02
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyAvailableAreaChanged02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    DisplayId displayId = 0;
    DMRect rect = {2, 2, 2, 2};

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    displayManagerAgentProxy->NotifyAvailableAreaChanged(rect, displayId);
    EXPECT_TRUE(g_logMsg.find("Write rect failed") != std::string::npos);
}

/**
 * @tc.name: OnScreenConnect02
 * @tc.desc: OnScreenConnect02
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, OnScreenConnect02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenInfo> screenInfo = sptr<ScreenInfo>::MakeSptr();

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->OnScreenConnect(screenInfo);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: OnScreenConnect03
 * @tc.desc: OnScreenConnect03
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, OnScreenConnect03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenInfo> screenInfo = sptr<ScreenInfo>::MakeSptr();

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    displayManagerAgentProxy->OnScreenConnect(screenInfo);
    EXPECT_TRUE(g_logMsg.find("Write ScreenInfo failed") != std::string::npos);
}

/**
 * @tc.name: OnScreenDisconnect01
 * @tc.desc: OnScreenDisconnect
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, OnScreenDisconnect01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenInfo> screenInfo = sptr<ScreenInfo>::MakeSptr();
    ScreenId screenId = 0;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->OnScreenDisconnect(screenId);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: OnScreenChange02
 * @tc.desc: OnScreenChange02
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, OnScreenChange02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenInfo> screenInfo = sptr<ScreenInfo>::MakeSptr();
    ScreenChangeEvent event = ScreenChangeEvent::CHANGE_MODE;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->OnScreenChange(screenInfo, event);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: OnScreenChange03
 * @tc.desc: OnScreenChange03
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, OnScreenChange03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenInfo> screenInfo = sptr<ScreenInfo>::MakeSptr();
    ScreenChangeEvent event = ScreenChangeEvent::CHANGE_MODE;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    displayManagerAgentProxy->OnScreenChange(screenInfo, event);
    EXPECT_TRUE(g_logMsg.find("Write screenInfo failed") != std::string::npos);
}

/**
 * @tc.name: OnScreenChange04
 * @tc.desc: OnScreenChange04
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, OnScreenChange04, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenInfo> screenInfo = sptr<ScreenInfo>::MakeSptr();
    ScreenChangeEvent event = ScreenChangeEvent::CHANGE_MODE;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    displayManagerAgentProxy->OnScreenChange(screenInfo, event);
    EXPECT_TRUE(g_logMsg.find("Write ScreenChangeEvent failed") != std::string::npos);
}

/**
 * @tc.name: OnScreenGroupChange02
 * @tc.desc: OnScreenGroupChange
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, OnScreenGroupChange02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::string trigger = " ";
    std::vector<sptr<ScreenInfo>> screenInfos = {};
    ScreenGroupChangeEvent event = ScreenGroupChangeEvent::CHANGE_GROUP;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->OnScreenGroupChange(trigger, screenInfos, event);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: OnScreenGroupChange03
 * @tc.desc: OnScreenGroupChange
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, OnScreenGroupChange03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::string trigger = " ";
    std::vector<sptr<ScreenInfo>> screenInfos = {};
    ScreenGroupChangeEvent event = ScreenGroupChangeEvent::CHANGE_GROUP;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    displayManagerAgentProxy->OnScreenGroupChange(trigger, screenInfos, event);
    EXPECT_TRUE(g_logMsg.find("Write ScreenGroupChangeEvent failed") != std::string::npos);
}

/**
 * @tc.name: OnDisplayCreate02
 * @tc.desc: OnDisplayCreate02
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, OnDisplayCreate02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->OnDisplayCreate(displayInfo);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: OnDisplayCreate03
 * @tc.desc: OnDisplayCreate03
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, OnDisplayCreate03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    displayManagerAgentProxy->OnDisplayCreate(displayInfo);
    EXPECT_TRUE(g_logMsg.find("Write DisplayInfo failed") != std::string::npos);
}

/**
 * @tc.name: OnDisplayDestroy01
 * @tc.desc: OnDisplayDestroy
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, OnDisplayDestroy01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    DisplayId displayId = 0;
    
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->OnDisplayDestroy(displayId);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: OnDisplayChange02
 * @tc.desc: OnDisplayChange02
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, OnDisplayChange02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    DisplayChangeEvent event = DisplayChangeEvent::DISPLAY_FREEZED;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->OnDisplayChange(displayInfo, event);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: OnDisplayChange03
 * @tc.desc: OnDisplayChange03
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, OnDisplayChange03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    DisplayChangeEvent event = DisplayChangeEvent::DISPLAY_FREEZED;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    displayManagerAgentProxy->OnDisplayChange(displayInfo, event);
    EXPECT_TRUE(g_logMsg.find("Write DisplayInfo failed") != std::string::npos);
}

/**
 * @tc.name: OnDisplayChange04
 * @tc.desc: OnDisplayChange04
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, OnDisplayChange04, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    DisplayChangeEvent event = DisplayChangeEvent::DISPLAY_FREEZED;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    displayManagerAgentProxy->OnDisplayChange(displayInfo, event);
    EXPECT_TRUE(g_logMsg.find("Write DisplayChangeEvent failed") != std::string::npos);
}

/**
 * @tc.name: OnScreenshot01
 * @tc.desc: OnScreenshot
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, OnScreenshot01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenshotInfo> snapshotInfo = sptr<ScreenshotInfo>::MakeSptr();

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->OnScreenshot(snapshotInfo);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: OnScreenshot02
 * @tc.desc: OnScreenshot
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, OnScreenshot02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<ScreenshotInfo> snapshotInfo = sptr<ScreenshotInfo>::MakeSptr();

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    displayManagerAgentProxy->OnScreenshot(snapshotInfo);
    EXPECT_TRUE(g_logMsg.find("Write ScreenshotInfo failed") != std::string::npos);
}

/**
 * @tc.name: NotifyRecordingDisplayChanged01
 * @tc.desc: NotifyRecordingDisplayChanged01
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyRecordingDisplayChanged01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::vector<DisplayId> displayIds;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->NotifyRecordingDisplayChanged(displayIds);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    displayManagerAgentProxy = sptr<DisplayManagerAgentProxy>::MakeSptr(remoteMocker);
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    displayManagerAgentProxy->NotifyRecordingDisplayChanged(displayIds);
    remoteMocker->SetRequestResult(ERR_NONE);
    displayManagerAgentProxy->NotifyRecordingDisplayChanged(displayIds);
    EXPECT_TRUE(g_logMsg.find("SendRequest failed") != std::string::npos);
}

/**
 * @tc.name: OnScreenshot02
 * @tc.desc: OnScreenshot
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyRecordingDisplayChanged02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::vector<DisplayId> displayIds;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint64VectorErrorFlag(true);
    displayManagerAgentProxy->NotifyRecordingDisplayChanged(displayIds);
    EXPECT_TRUE(g_logMsg.find("Write displayId failed") != std::string::npos);
}

/**
 * @tc.name: NotifyPrivateWindowStateChanged01
 * @tc.desc: NotifyPrivateWindowStateChanged01
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyPrivateWindowStateChanged01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    bool hasPrivate = true;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->NotifyPrivateWindowStateChanged(hasPrivate);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyPrivateWindowStateChanged02
 * @tc.desc: NotifyPrivateWindowStateChanged02
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyPrivateWindowStateChanged02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    bool hasPrivate = true;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    displayManagerAgentProxy->NotifyPrivateWindowStateChanged(hasPrivate);
    EXPECT_TRUE(g_logMsg.find("Write private info failed") != std::string::npos);
}

/**
 * @tc.name: NotifyPrivateStateWindowListChanged01
 * @tc.desc: NotifyPrivateStateWindowListChanged
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyPrivateStateWindowListChanged01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    DisplayId id = 0;
    std::vector<std::string> privacyWindowList{"win0", "win1"};

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->NotifyPrivateStateWindowListChanged(id, privacyWindowList);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyFoldStatusChanged
 * @tc.desc: NotifyFoldStatusChanged
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyFoldStatusChanged, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerAgentProxy->NotifyFoldStatusChanged(FoldStatus::EXPAND);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyFoldStatusChanged01
 * @tc.desc: NotifyFoldStatusChanged
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerAgentProxyTest, NotifyFoldStatusChanged01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    displayManagerAgentProxy->NotifyFoldStatusChanged(FoldStatus::EXPAND);
    EXPECT_TRUE(g_logMsg.find("Write foldStatus failed") != std::string::npos);
}
}