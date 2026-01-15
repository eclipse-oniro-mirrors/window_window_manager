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

#include <functional>
#include <gtest/gtest.h>

#include "scene_board_judgement.h"
#include "iremote_object_mocker.h"
#include "window_manager_agent_proxy.h"
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

namespace OHOS {
namespace Rosen {
class WindowManagerAgentProxyTest : public testing::Test {
public:
    static void SetUpTestSuite();
    void SetUp() override;
    void TearDown() override;
    sptr<WindowManagerAgentProxy> windowManagerAgentProxy;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void WindowManagerAgentProxyTest::SetUpTestSuite()
{
}

void WindowManagerAgentProxyTest::SetUp()
{
    if (windowManagerAgentProxy) {
        return;
    }

    sptr<IRemoteObject> impl;
    impl = sptr<IRemoteObjectMocker>::MakeSptr();

    windowManagerAgentProxy = sptr<WindowManagerAgentProxy>::MakeSptr(impl);
    ASSERT_NE(windowManagerAgentProxy, nullptr);
}

void WindowManagerAgentProxyTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name: UpdateFocusChangeInfo01
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateFocusChangeInfo01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<FocusChangeInfo> focusChangeInfo = sptr<FocusChangeInfo>::MakeSptr();
    ASSERT_TRUE(focusChangeInfo != nullptr);
    bool focused = true;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowManagerAgentProxy->UpdateFocusChangeInfo(focusChangeInfo, focused);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: UpdateFocusChangeInfo02
 * @tc.desc: test write focusChangeInfo failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateFocusChangeInfo02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<FocusChangeInfo> focusChangeInfo = new(std::nothrow) FocusChangeInfo();
    ASSERT_TRUE(focusChangeInfo != nullptr);
    bool focused = true;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    windowManagerAgentProxy->UpdateFocusChangeInfo(focusChangeInfo, focused);
    EXPECT_TRUE(g_logMsg.find("Write displayId failed") != std::string::npos);
}

/**
 * @tc.name: UpdateFocusChangeInfo03
 * @tc.desc: test write focused failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateFocusChangeInfo03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<FocusChangeInfo> focusChangeInfo = new(std::nothrow) FocusChangeInfo();
    ASSERT_TRUE(focusChangeInfo != nullptr);
    bool focused = true;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    windowManagerAgentProxy->UpdateFocusChangeInfo(focusChangeInfo, focused);
    EXPECT_TRUE(g_logMsg.find("Write Focus failed") != std::string::npos);
}

/**
 * @tc.name: UpdateFocusChangeInfo04
 * @tc.desc: test focusChangeInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateFocusChangeInfo04, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    bool focused = true;

    MockMessageParcel::ClearAllErrorFlag();
    windowManagerAgentProxy->UpdateFocusChangeInfo(nullptr, focused);
    EXPECT_TRUE(g_logMsg.find("Invalid focus change info") != std::string::npos);
}

/**
 * @tc.name: UpdateWindowModeTypeInfo01
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateWindowModeTypeInfo01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    WindowModeType type = WindowModeType::WINDOW_MODE_SPLIT_FLOATING;
    
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowManagerAgentProxy->UpdateWindowModeTypeInfo(type);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: UpdateSystemBarRegionTints01
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateSystemBarRegionTints01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    DisplayId displayId = 0;
    SystemBarRegionTints tints = {};
    
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowManagerAgentProxy->UpdateSystemBarRegionTints(displayId, tints);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: UpdateSystemBarRegionTints02
 * @tc.desc: test write tints and displayId failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateSystemBarRegionTints02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    DisplayId displayId = 0;
    SystemBarRegionTints tints = {};

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    windowManagerAgentProxy->UpdateSystemBarRegionTints(displayId, tints);
    EXPECT_TRUE(g_logMsg.find("Write displayId failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    windowManagerAgentProxy->UpdateSystemBarRegionTints(displayId, tints);
    EXPECT_TRUE(g_logMsg.find("Write SystemBarRegionTint failed") != std::string::npos);
}

/**
 * @tc.name: NotifyAccessibilityWindowInfo
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifyAccessibilityWindowInfo, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::vector<sptr<AccessibilityWindowInfo>> infos = {};
    WindowUpdateType type = WindowUpdateType::WINDOW_UPDATE_REMOVED;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowManagerAgentProxy->NotifyAccessibilityWindowInfo(infos, type);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyAccessibilityWindowInfo01
 * @tc.desc: test write infos failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifyAccessibilityWindowInfo01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    sptr<AccessibilityWindowInfo> info = sptr<AccessibilityWindowInfo>::MakeSptr();
    infos.push_back(info);
    WindowUpdateType type = WindowUpdateType::WINDOW_UPDATE_REMOVED;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    windowManagerAgentProxy->NotifyAccessibilityWindowInfo(infos, type);
    EXPECT_TRUE(g_logMsg.find("Write accessibility window infos failed") != std::string::npos);
}

/**
 * @tc.name: UpdateWindowVisibilityInfo01
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateWindowVisibilityInfo01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::vector<sptr<WindowVisibilityInfo>> visibilityInfos = {};
    
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowManagerAgentProxy->UpdateWindowVisibilityInfo(visibilityInfos);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: UpdateWindowVisibilityInfo02
 * @tc.desc: test write visibilityInfos failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateWindowVisibilityInfo02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::vector<sptr<WindowVisibilityInfo>> visibilityInfos = {};
    
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    windowManagerAgentProxy->UpdateWindowVisibilityInfo(visibilityInfos);
    EXPECT_TRUE(g_logMsg.find("write windowVisibilityInfos size failed") != std::string::npos);
}

/**
 * @tc.name: UpdateWindowVisibilityInfo03
 * @tc.desc: test write visibilityInfos failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateWindowVisibilityInfo03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::vector<sptr<WindowVisibilityInfo>> visibilityInfos = {};
    sptr<WindowVisibilityInfo> visibilityInfo = sptr<WindowVisibilityInfo>::MakeSptr();
    visibilityInfos.push_back(visibilityInfo);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    windowManagerAgentProxy->UpdateWindowVisibilityInfo(visibilityInfos);
    EXPECT_TRUE(g_logMsg.find("Write windowVisibilityInfo failed") != std::string::npos);
}

/**
 * @tc.name: UpdateWindowDrawingContentInfo01
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateWindowDrawingContentInfo01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::vector<sptr<WindowDrawingContentInfo>> windowDrawingContentInfos = {};
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowManagerAgentProxy->UpdateWindowDrawingContentInfo(windowDrawingContentInfos);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: UpdateWindowDrawingContentInfo02
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateWindowDrawingContentInfo02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::vector<sptr<WindowDrawingContentInfo>> windowDrawingContentInfos = {};
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    windowManagerAgentProxy->UpdateWindowDrawingContentInfo(windowDrawingContentInfos);
    EXPECT_TRUE(g_logMsg.find("write windowDrawingContentInfos size failed") != std::string::npos);
}

/**
 * @tc.name: UpdateWindowDrawingContentInfo03
 * @tc.desc: test wirte windowDrawingContentInfos failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateWindowDrawingContentInfo03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::vector<sptr<WindowDrawingContentInfo>> windowDrawingContentInfos = {};
    sptr<WindowDrawingContentInfo> windowDrawingContentInfo = sptr<WindowDrawingContentInfo>::MakeSptr();
    windowDrawingContentInfos.push_back(windowDrawingContentInfo);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    windowManagerAgentProxy->UpdateWindowDrawingContentInfo(windowDrawingContentInfos);
    EXPECT_TRUE(g_logMsg.find("Write windowDrawingContentInfos failed") != std::string::npos);
}

/**
 * @tc.name: UpdateCameraFloatWindowStatus
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateCameraFloatWindowStatus, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    uint32_t accessTokenId = 0;
    bool isShowing = true;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowManagerAgentProxy->UpdateCameraFloatWindowStatus(accessTokenId, isShowing);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: UpdateCameraFloatWindowStatus01
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateCameraFloatWindowStatus01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    uint32_t accessTokenId = 0;
    bool isShowing = true;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    windowManagerAgentProxy->UpdateCameraFloatWindowStatus(accessTokenId, isShowing);
    EXPECT_TRUE(g_logMsg.find("Write accessTokenId failed") != std::string::npos);
}

/**
 * @tc.name: UpdateCameraFloatWindowStatus02
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateCameraFloatWindowStatus02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    uint32_t accessTokenId = 0;
    bool isShowing = true;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    windowManagerAgentProxy->UpdateCameraFloatWindowStatus(accessTokenId, isShowing);
    EXPECT_TRUE(g_logMsg.find("Write is showing status failed") != std::string::npos);
}

/**
 * @tc.name: NotifyWaterMarkFlagChangedResult
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifyWaterMarkFlagChangedResult, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    bool showWaterMark = true;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowManagerAgentProxy->NotifyWaterMarkFlagChangedResult(showWaterMark);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyWaterMarkFlagChangedResult01
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifyWaterMarkFlagChangedResult01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    bool showWaterMark = true;
    
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    windowManagerAgentProxy->NotifyWaterMarkFlagChangedResult(showWaterMark);
    EXPECT_TRUE(g_logMsg.find("Write is showing status failed") != std::string::npos);
}

/**
 * @tc.name: UpdateVisibleWindowNum
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateVisibleWindowNum, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    VisibleWindowNumInfo info;
    info.displayId = 1;
    info.visibleWindowNum = 1;
    std::vector<VisibleWindowNumInfo> visibleWindowNumInfo;
    visibleWindowNumInfo.push_back(info);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowManagerAgentProxy->UpdateVisibleWindowNum(visibleWindowNumInfo);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: UpdateVisibleWindowNum01
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateVisibleWindowNum01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    VisibleWindowNumInfo info;
    info.displayId = 1;
    info.visibleWindowNum = 1;
    std::vector<VisibleWindowNumInfo> visibleWindowNumInfo;
    visibleWindowNumInfo.push_back(info);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    windowManagerAgentProxy->UpdateVisibleWindowNum(visibleWindowNumInfo);
    EXPECT_TRUE(g_logMsg.find("Write VisibleWindowNumInfo failed") != std::string::npos);
}

/**
 * @tc.name: NotifyGestureNavigationEnabledResult
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifyGestureNavigationEnabledResult, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    bool enable = true;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowManagerAgentProxy->NotifyGestureNavigationEnabledResult(enable);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyGestureNavigationEnabledResult01
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifyGestureNavigationEnabledResult01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    bool enable = true;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    windowManagerAgentProxy->NotifyGestureNavigationEnabledResult(enable);
    EXPECT_TRUE(g_logMsg.find("Write is showing status failed") != std::string::npos);
}


/**
 * @tc.name: UpdateCameraWindowStatus
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateCameraWindowStatus, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    uint32_t accessTokenId = 1;
    bool isShowing = false;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowManagerAgentProxy->UpdateCameraWindowStatus(accessTokenId, isShowing);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: UpdateCameraWindowStatus01
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateCameraWindowStatus01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    uint32_t accessTokenId = 1;
    bool isShowing = false;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    windowManagerAgentProxy->UpdateCameraWindowStatus(accessTokenId, isShowing);
    EXPECT_TRUE(g_logMsg.find("Write accessTokenId failed") != std::string::npos);
}

/**
 * @tc.name: UpdateCameraWindowStatus02
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdateCameraWindowStatus02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    uint32_t accessTokenId = 1;
    bool isShowing = false;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    windowManagerAgentProxy->UpdateCameraWindowStatus(accessTokenId, isShowing);
    EXPECT_TRUE(g_logMsg.find("Write isShowing status failed") != std::string::npos);
}

/**
 * @tc.name: NotifyWindowStyleChange
 * @tc.desc: test NotifyWindowStyleChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifyWindowStyleChange, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    WindowStyleType type = Rosen::WindowStyleType::WINDOW_STYLE_DEFAULT;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowManagerAgentProxy->NotifyWindowStyleChange(type);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyCallingWindowDisplayChanged01
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifyCallingWindowDisplayChanged01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    CallingWindowInfo windowInfo;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowManagerAgentProxy->NotifyCallingWindowDisplayChanged(windowInfo);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyCallingWindowDisplayChanged02
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifyCallingWindowDisplayChanged02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    CallingWindowInfo windowInfo;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    windowManagerAgentProxy->NotifyCallingWindowDisplayChanged(windowInfo);
    EXPECT_TRUE(g_logMsg.find("Write callingWindowInfo failed") != std::string::npos);
}

/**
 * @tc.name: NotifyWindowSystemBarPropertyChange
 * @tc.desc: test NotifyWindowSystemBarPropertyChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifyWindowSystemBarPropertyChange, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    SystemBarProperty systemBarProperty;
    windowManagerAgentProxy->NotifyWindowSystemBarPropertyChange(
        WindowType::WINDOW_TYPE_STATUS_BAR, systemBarProperty);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyWindowPidVisibilityChanged
 * @tc.desc: test NotifyWindowPidVisibilityChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifyWindowPidVisibilityChanged, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowPidVisibilityInfo> info = sptr<WindowPidVisibilityInfo>::MakeSptr();

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowManagerAgentProxy->NotifyWindowPidVisibilityChanged(info);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyWindowPidVisibilityChanged01
 * @tc.desc: test NotifyWindowPidVisibilityChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifyWindowPidVisibilityChanged01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<WindowPidVisibilityInfo> info = sptr<WindowPidVisibilityInfo>::MakeSptr();

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    windowManagerAgentProxy->NotifyWindowPidVisibilityChanged(info);
    EXPECT_TRUE(g_logMsg.find("Write windowPidVisibilityInfo failed") != std::string::npos);
}

/**
 * @tc.name: NotifyWindowPidVisibilityChanged02
 * @tc.desc: test NotifyWindowPidVisibilityChanged02
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifyWindowPidVisibilityChanged02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    windowManagerAgentProxy->NotifyWindowPidVisibilityChanged(nullptr);
    EXPECT_TRUE(g_logMsg.find("Invalid window pid visibility info") != std::string::npos);
}

/**
 * @tc.name: UpdatePiPWindowStateChanged
 * @tc.desc: test UpdatePiPWindowStateChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, UpdatePiPWindowStateChanged, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::string bundleName = "test";
    bool isForeground = false;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowManagerAgentProxy->UpdatePiPWindowStateChanged(bundleName, isForeground);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyWindowPropertyChange
 * @tc.desc: test NotifyWindowPropertyChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifyWindowPropertyChange, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    uint32_t propertyDirtyFlags = 0;
    WindowInfoList windowInfoList;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowManagerAgentProxy->NotifyWindowPropertyChange(propertyDirtyFlags, windowInfoList);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
}

/**
 * @tc.name: NotifyWindowPropertyChange01
 * @tc.desc: test NotifyWindowPropertyChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifyWindowPropertyChange01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    uint32_t propertyDirtyFlags = 0;
    WindowInfoList windowInfoList;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    windowManagerAgentProxy->NotifyWindowPropertyChange(propertyDirtyFlags, windowInfoList);
    EXPECT_TRUE(g_logMsg.find("Write propertyDirtyFlags failed") != std::string::npos);
}

/**
 * @tc.name: NotifyWindowPropertyChange02
 * @tc.desc: test NotifyWindowPropertyChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifyWindowPropertyChange02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    uint32_t propertyDirtyFlags = 0;
    WindowInfoList windowInfoList;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> info = {
        { WindowInfoKey::APP_INDEX, 0 },
    };
    windowInfoList.push_back(info);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    windowManagerAgentProxy->NotifyWindowPropertyChange(propertyDirtyFlags, windowInfoList);
    EXPECT_TRUE(g_logMsg.find("Write window change info value failed") != std::string::npos);
}

/**
 * @tc.name: WriteWindowChangeInfoValue
 * @tc.desc: test WriteWindowChangeInfoValue
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, WriteWindowChangeInfoValue, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MessageParcel data;
    std::pair<WindowInfoKey, WindowChangeInfoType> windowInfoPair;

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_TRUE(g_logMsg.find("Write windowInfoKey failed") != std::string::npos);
}

/**
 * @tc.name: WriteWindowChangeInfoValue01
 * @tc.desc: test WriteWindowChangeInfoValue
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, WriteWindowChangeInfoValue01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();
    MessageParcel data;
    std::pair<WindowInfoKey, WindowChangeInfoType> windowInfoPair;

    WindowChangeInfoType windowInfo = 0;
    windowInfoPair = std::make_pair(WindowInfoKey::WINDOW_ID, windowInfo);
    bool ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, true);

    windowInfo = std::string("test");
    windowInfoPair = std::make_pair(WindowInfoKey::BUNDLE_NAME, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, true);

    windowInfoPair = std::make_pair(WindowInfoKey::ABILITY_NAME, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, true);

    windowInfo = 0;
    windowInfoPair = std::make_pair(WindowInfoKey::APP_INDEX, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, true);

    windowInfo = WindowVisibilityState::START;
    windowInfoPair = std::make_pair(WindowInfoKey::VISIBILITY_STATE, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, true);

    windowInfo = static_cast<uint64_t>(0);
    windowInfoPair = std::make_pair(WindowInfoKey::DISPLAY_ID, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, true);

    windowInfo = Rect({0, 0, 0, 0});
    windowInfoPair = std::make_pair(WindowInfoKey::WINDOW_RECT, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, true);

    windowInfo = WindowMode::WINDOW_MODE_FULLSCREEN;
    windowInfoPair = std::make_pair(WindowInfoKey::WINDOW_MODE, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, true);

    windowInfo = 1.0f;
    windowInfoPair = std::make_pair(WindowInfoKey::FLOATING_SCALE, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, true);

    windowInfo = Rect({0, 0, 0, 0});
    windowInfoPair = std::make_pair(WindowInfoKey::WINDOW_GLOBAL_RECT, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, true);

    windowInfo = 0;
    windowInfoPair = std::make_pair(WindowInfoKey::NONE, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: WriteWindowChangeInfoValue02
 * @tc.desc: test WriteWindowChangeInfoValue fail 1
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, WriteWindowChangeInfoValue02, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();
    MessageParcel data;
    std::pair<WindowInfoKey, WindowChangeInfoType> windowInfoPair;

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    WindowChangeInfoType windowInfo = 0;
    windowInfoPair = std::make_pair(WindowInfoKey::WINDOW_ID, windowInfo);
    bool ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, false);

    MockMessageParcel::SetWriteStringErrorFlag(true);
    windowInfo = std::string("test");
    windowInfoPair = std::make_pair(WindowInfoKey::BUNDLE_NAME, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, false);

    windowInfoPair = std::make_pair(WindowInfoKey::ABILITY_NAME, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, false);

    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    windowInfo = 0;
    windowInfoPair = std::make_pair(WindowInfoKey::APP_INDEX, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, false);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    windowInfo = WindowVisibilityState::START;
    windowInfoPair = std::make_pair(WindowInfoKey::VISIBILITY_STATE, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: WriteWindowChangeInfoValue03
 * @tc.desc: test WriteWindowChangeInfoValue fail 2
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, WriteWindowChangeInfoValue03, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();
    MessageParcel data;
    std::pair<WindowInfoKey, WindowChangeInfoType> windowInfoPair;

    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    WindowChangeInfoType windowInfo = static_cast<uint64_t>(0);
    windowInfoPair = std::make_pair(WindowInfoKey::DISPLAY_ID, windowInfo);
    auto ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, false);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    windowInfo = Rect({0, 0, 0, 0});
    windowInfoPair = std::make_pair(WindowInfoKey::WINDOW_RECT, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, false);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    windowInfo = WindowMode::WINDOW_MODE_FULLSCREEN;
    windowInfoPair = std::make_pair(WindowInfoKey::WINDOW_MODE, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, false);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    windowInfo = Rect({0, 0, 0, 0});
    windowInfoPair = std::make_pair(WindowInfoKey::WINDOW_GLOBAL_RECT, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, false);

    MockMessageParcel::SetWriteFloatErrorFlag(true);
    windowInfo = 1.0f;
    windowInfoPair = std::make_pair(WindowInfoKey::FLOATING_SCALE, windowInfo);
    ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, false);

    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: WriteWindowChangeInfoValue04_MidScene_Write_Success
 * @tc.desc: test WriteWindowChangeInfoValue for MID_SCENE write success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, WriteWindowChangeInfoValue04_MidScene_Write_Success, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();
    MessageParcel data;
    std::pair<WindowInfoKey, WindowChangeInfoType> windowInfoPair;
    WindowChangeInfoType windowInfo = true;
    windowInfoPair = std::make_pair(WindowInfoKey::MID_SCENE, windowInfo);
    auto ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: WriteWindowChangeInfoValue04_MidScene_Write_Failed
 * @tc.desc: test WriteWindowChangeInfoValue for MID_SCENE write failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, WriteWindowChangeInfoValue04_MidScene_Write_Failed, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();
    MessageParcel data;
    std::pair<WindowInfoKey, WindowChangeInfoType> windowInfoPair;

    MockMessageParcel::SetWriteBoolErrorFlag(true);
    WindowChangeInfoType windowInfo = true;
    windowInfoPair = std::make_pair(WindowInfoKey::MID_SCENE, windowInfo);
    auto ret = windowManagerAgentProxy->WriteWindowChangeInfoValue(data, windowInfoPair);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: NotifySupportRotationChange
 * @tc.desc: test NotifySupportRotationChange01
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifySupportRotationChange01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();
    MessageParcel data;

    SupportRotationInfo supportRotationInfo;
    supportRotationInfo.displayId_ = 0;
    supportRotationInfo.persistentId_ = 0;
    supportRotationInfo.containerSupportRotation_ = {false, false, false, false};
    supportRotationInfo.sceneSupportRotation_ = {false, false, false, false};
    supportRotationInfo.supportRotationChangeReason_ = "test";

    // remote == nullptr
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    MockMessageParcel::SetWriteParcelableErrorFlag(false);
    auto tempProxy = sptr<WindowManagerAgentProxy>::MakeSptr(nullptr);
    tempProxy->NotifySupportRotationChange(supportRotationInfo);
    EXPECT_TRUE(g_logMsg.find("remote is null") != std::string::npos);
}

/**
 * @tc.name: NotifySupportRotationChange
 * @tc.desc: test NotifySupportRotationChange02
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifySupportRotationChange02, TestSize.Level1)
{
    g_logMsg.clear();
    SupportRotationInfo supportRotationInfo;
    supportRotationInfo.displayId_ = 0;
    supportRotationInfo.persistentId_ = 0;
    supportRotationInfo.containerSupportRotation_ = {false, false, false, false};
    supportRotationInfo.sceneSupportRotation_ = {false, false, false, false};
    supportRotationInfo.supportRotationChangeReason_ = "test";

    // WriteInterfaceToken failed
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ASSERT_NE(windowManagerAgentProxy, nullptr);
    windowManagerAgentProxy->NotifySupportRotationChange(supportRotationInfo);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
}

/**
 * @tc.name: NotifySupportRotationChange
 * @tc.desc: test NotifySupportRotationChange03
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerAgentProxyTest, NotifySupportRotationChange03, TestSize.Level1)
{
    g_logMsg.clear();
    SupportRotationInfo supportRotationInfo;
    supportRotationInfo.displayId_ = 0;
    supportRotationInfo.persistentId_ = 0;
    supportRotationInfo.containerSupportRotation_ = {false, false, false, false};
    supportRotationInfo.sceneSupportRotation_ = {false, false, false, false};
    supportRotationInfo.supportRotationChangeReason_ = "test";

    // WriteParcelable failed
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    windowManagerAgentProxy->NotifySupportRotationChange(supportRotationInfo);
    EXPECT_TRUE(g_logMsg.find("supportRotationInfo marshalling failed") != std::string::npos);
    MockMessageParcel::SetWriteParcelableErrorFlag(false);
    MockMessageParcel::ClearAllErrorFlag();

    windowManagerAgentProxy->NotifySupportRotationChange(supportRotationInfo);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
