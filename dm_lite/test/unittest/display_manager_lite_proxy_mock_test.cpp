/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "display_manager_agent_default.h"
#include "display_manager_lite_proxy.h"
#include "iremote_object_mocker.h"
#include "mock_message_parcel.h"

using namespace testing;
using namespace testing::ext;

namespace {
std::string logMsg;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
    const char* msg)
{
    logMsg = msg;
}
}

namespace OHOS ::Rosen {
class DisplayManagerLiteProxyMockTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    sptr<DisplayManagerLiteProxy> displayManagerLiteProxy_;
};

void DisplayManagerLiteProxyMockTest::SetUp()
{
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    displayManagerLiteProxy_ = sptr<DisplayManagerLiteProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(displayManagerLiteProxy_, nullptr);
}

void DisplayManagerLiteProxyMockTest::TearDown()
{
    displayManagerLiteProxy_ = nullptr;
}

namespace {
/**
 * @tc.name: SetResolution
 * @tc.desc: SetResolution
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, SetResolution, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenId id = 1;
    uint32_t width = 1080;
    uint32_t height = 2400;
    float vpr = 2.8f;

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("WriteInterfaceToken failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    displayManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("write screenId/width/height/virtualPixelRatio failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    displayManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("write screenId/width/height/virtualPixelRatio failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    displayManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("write screenId/width/height/virtualPixelRatio failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteFloatErrorFlag(true);
    displayManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("write screenId/width/height/virtualPixelRatio failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    displayManagerLiteProxy_->SetResolution(id, width, height, vpr);
    EXPECT_TRUE(logMsg.find("SendRequest failed") == std::string::npos);
}

/**
 * @tc.name: SetScreenSwitchState01
 * @tc.desc: SetScreenSwitchState with remote nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, SetScreenSwitchState01, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(nullptr);
    ScreenClosedState screenClosedState = ScreenClosedState::CLOSE;
    bool isScreenOn = false;
    DMError ret = proxy->SetScreenSwitchState(screenClosedState, isScreenOn);
    EXPECT_EQ(ret, DMError::DM_ERROR_NULLPTR);
    EXPECT_TRUE(logMsg.find("remote is nullptr") != std::string::npos);
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetScreenSwitchState02
 * @tc.desc: SetScreenSwitchState with WriteInterfaceToken failed
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, SetScreenSwitchState02, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ScreenClosedState screenClosedState = ScreenClosedState::CLOSE;
    bool isScreenOn = false;
    DMError ret = proxy->SetScreenSwitchState(screenClosedState, isScreenOn);
    EXPECT_EQ(ret, DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED);
    EXPECT_TRUE(logMsg.find("WriteInterfaceToken failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetScreenSwitchState03
 * @tc.desc: SetScreenSwitchState with WriteUint32 screenClosedState failed
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, SetScreenSwitchState03, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    ScreenClosedState screenClosedState = ScreenClosedState::OPEN;
    bool isScreenOn = true;
    DMError ret = proxy->SetScreenSwitchState(screenClosedState, isScreenOn);
    EXPECT_EQ(ret, DMError::DM_ERROR_WRITE_DATA_FAILED);
    EXPECT_TRUE(logMsg.find("Write screenClosedState failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetScreenSwitchState04
 * @tc.desc: SetScreenSwitchState with WriteBool isScreenOn failed
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, SetScreenSwitchState04, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::SetWriteBoolErrorFlag(true);
    ScreenClosedState screenClosedState = ScreenClosedState::CLOSE;
    bool isScreenOn = false;
    DMError ret = proxy->SetScreenSwitchState(screenClosedState, isScreenOn);
    EXPECT_EQ(ret, DMError::DM_ERROR_WRITE_DATA_FAILED);
    EXPECT_TRUE(logMsg.find("Write isScreenOn failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetScreenSwitchState05
 * @tc.desc: SetScreenSwitchState with SendRequest failed
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, SetScreenSwitchState05, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ScreenClosedState screenClosedState = ScreenClosedState::OPEN;
    bool isScreenOn = true;
    DMError ret = proxy->SetScreenSwitchState(screenClosedState, isScreenOn);
    EXPECT_EQ(ret, DMError::DM_ERROR_IPC_FAILED);
    EXPECT_TRUE(logMsg.find("SendRequest failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    remoteMocker->SetRequestResult(ERR_NONE);
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetScreenSwitchState06
 * @tc.desc: SetScreenSwitchState successfully with CLOSE state and screen off
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, SetScreenSwitchState06, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    remoteMocker->SetRequestResult(ERR_NONE);
    ScreenClosedState screenClosedState = ScreenClosedState::CLOSE;
    bool isScreenOn = false;
    DMError ret = proxy->SetScreenSwitchState(screenClosedState, isScreenOn);
    EXPECT_EQ(ret, DMError::DM_OK);
    EXPECT_TRUE(logMsg.find("SendRequest failed") == std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: IsOnboardDisplay
 * @tc.desc: IsOnboardDisplay
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, IsOnboardDisplay, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    DisplayId displayId = 0;
    bool isOnboardDisplay = false;
    MockMessageParcel::ClearAllErrorFlag();

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerLiteProxy_->IsOnboardDisplay(displayId, isOnboardDisplay);
    EXPECT_TRUE(logMsg.find("write interface token failed") != std::string::npos);

    logMsg.clear();
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    displayManagerLiteProxy_->IsOnboardDisplay(displayId, isOnboardDisplay);
    EXPECT_TRUE(logMsg.find("write displayId failed") != std::string::npos);

    logMsg.clear();
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    proxy->IsOnboardDisplay(displayId, isOnboardDisplay);
    EXPECT_TRUE(logMsg.find("send request failed") != std::string::npos);
    remoteMocker->SetRequestResult(ERR_NONE);

    logMsg.clear();
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    displayManagerLiteProxy_->IsOnboardDisplay(displayId, isOnboardDisplay);
    EXPECT_TRUE(logMsg.find("read result failed") != std::string::npos);
    MockMessageParcel::SetReadInt32ErrorFlag(false);

    logMsg.clear();
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetReadBoolErrorFlag(true);
    remoteMocker->SetRequestResult(ERR_NONE);
    DMError ret = displayManagerLiteProxy_->IsOnboardDisplay(displayId, isOnboardDisplay);
    EXPECT_EQ(ret, DMError::DM_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadBoolErrorFlag(false);

    logMsg.clear();
    MockMessageParcel::ClearAllErrorFlag();
    remoteMocker = nullptr;
    proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    proxy->IsOnboardDisplay(displayId, isOnboardDisplay);
    EXPECT_TRUE(logMsg.find("remote is null") != std::string::npos);
    MockMessageParcel::ClearAllErrorFlag();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: RegisterDisplayAttributeAgent01
 * @tc.desc: RegisterDisplayAttributeAgent with remote nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, RegisterDisplayAttributeAgent01, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(nullptr);
    std::vector<std::string> attributes = { "rotation" };
    sptr<IDisplayManagerAgent> agent = nullptr;
    DMError ret = proxy->RegisterDisplayAttributeAgent(attributes, agent);
    EXPECT_EQ(ret, DMError::DM_ERROR_IPC_FAILED);
    EXPECT_TRUE(logMsg.find("remote is null") != std::string::npos);
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: RegisterDisplayAttributeAgent02
 * @tc.desc: RegisterDisplayAttributeAgent with WriteInterfaceToken failed
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, RegisterDisplayAttributeAgent02, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    std::vector<std::string> attributes = { "rotation" };
    sptr<IDisplayManagerAgent> agent = nullptr;
    DMError ret = proxy->RegisterDisplayAttributeAgent(attributes, agent);
    EXPECT_EQ(ret, DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED);
    EXPECT_TRUE(logMsg.find("WriteInterfaceToken failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: RegisterDisplayAttributeAgent03
 * @tc.desc: RegisterDisplayAttributeAgent with agent nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, RegisterDisplayAttributeAgent03, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    std::vector<std::string> attributes = { "rotation" };
    sptr<IDisplayManagerAgent> agent = nullptr;
    DMError ret = proxy->RegisterDisplayAttributeAgent(attributes, agent);
    EXPECT_EQ(ret, DMError::DM_ERROR_INVALID_PARAM);
    EXPECT_TRUE(logMsg.find("IDisplayManagerAgent is null") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: RegisterDisplayAttributeAgent04
 * @tc.desc: RegisterDisplayAttributeAgent with WriteStringVector failed
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, RegisterDisplayAttributeAgent04, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::SetWriteStringVectorErrorFlag(true);
    std::vector<std::string> attributes = { "rotation" };
    sptr<IDisplayManagerAgent> agent = new DisplayManagerAgentDefault();
    DMError ret = proxy->RegisterDisplayAttributeAgent(attributes, agent);
    EXPECT_EQ(ret, DMError::DM_ERROR_IPC_FAILED);
    EXPECT_TRUE(logMsg.find("Write attributes failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: RegisterDisplayAttributeAgent05
 * @tc.desc: RegisterDisplayAttributeAgent with SendRequest failed
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, RegisterDisplayAttributeAgent05, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    std::vector<std::string> attributes = { "rotation" };
    sptr<IDisplayManagerAgent> agent = new DisplayManagerAgentDefault();
    DMError ret = proxy->RegisterDisplayAttributeAgent(attributes, agent);
    EXPECT_EQ(ret, DMError::DM_ERROR_IPC_FAILED);
    EXPECT_TRUE(logMsg.find("SendRequest failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    remoteMocker->SetRequestResult(ERR_NONE);
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: RegisterDisplayAttributeAgent06
 * @tc.desc: RegisterDisplayAttributeAgent successfully
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, RegisterDisplayAttributeAgent06, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    remoteMocker->SetRequestResult(ERR_NONE);
    std::vector<std::string> attributes = { "rotation" };
    sptr<IDisplayManagerAgent> agent = new DisplayManagerAgentDefault();
    DMError ret = proxy->RegisterDisplayAttributeAgent(attributes, agent);
    EXPECT_EQ(ret, DMError::DM_OK);
    EXPECT_TRUE(logMsg.find("SendRequest failed") == std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: UnRegisterDisplayAttribute01
 * @tc.desc: UnregisterDisplayAttribute with remote nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, UnRegisterDisplayAttribute01, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(nullptr);
    std::vector<std::string> attributes = { "rotation" };
    sptr<IDisplayManagerAgent> agent = nullptr;
    DMError ret = proxy->UnregisterDisplayAttribute(attributes, agent);
    EXPECT_EQ(ret, DMError::DM_ERROR_IPC_FAILED);
    EXPECT_TRUE(logMsg.find("remote is null") != std::string::npos);
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: UnRegisterDisplayAttribute02
 * @tc.desc: UnregisterDisplayAttribute with WriteInterfaceToken failed
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, UnRegisterDisplayAttribute02, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    std::vector<std::string> attributes = { "rotation" };
    sptr<IDisplayManagerAgent> agent = nullptr;
    DMError ret = proxy->UnregisterDisplayAttribute(attributes, agent);
    EXPECT_EQ(ret, DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED);
    EXPECT_TRUE(logMsg.find("WriteInterfaceToken failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: UnRegisterDisplayAttribute03
 * @tc.desc: UnregisterDisplayAttribute with agent nullptr
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, UnRegisterDisplayAttribute03, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    std::vector<std::string> attributes = { "rotation" };
    sptr<IDisplayManagerAgent> agent = nullptr;
    DMError ret = proxy->UnregisterDisplayAttribute(attributes, agent);
    EXPECT_EQ(ret, DMError::DM_ERROR_INVALID_PARAM);
    EXPECT_TRUE(logMsg.find("IDisplayManagerAgent is null") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: UnRegisterDisplayAttribute04
 * @tc.desc: UnregisterDisplayAttribute with WriteStringVector failed
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, UnRegisterDisplayAttribute04, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::SetWriteStringVectorErrorFlag(true);
    std::vector<std::string> attributes = { "rotation" };
    sptr<IDisplayManagerAgent> agent = new DisplayManagerAgentDefault();
    DMError ret = proxy->UnregisterDisplayAttribute(attributes, agent);
    EXPECT_EQ(ret, DMError::DM_ERROR_IPC_FAILED);
    EXPECT_TRUE(logMsg.find("Write attributes failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: UnRegisterDisplayAttribute05
 * @tc.desc: UnregisterDisplayAttribute with SendRequest failed
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, UnRegisterDisplayAttribute05, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    std::vector<std::string> attributes = { "rotation" };
    sptr<IDisplayManagerAgent> agent = new DisplayManagerAgentDefault();
    DMError ret = proxy->UnregisterDisplayAttribute(attributes, agent);
    EXPECT_EQ(ret, DMError::DM_ERROR_IPC_FAILED);
    EXPECT_TRUE(logMsg.find("SendRequest failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    remoteMocker->SetRequestResult(ERR_NONE);
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: UnRegisterDisplayAttribute06
 * @tc.desc: UnregisterDisplayAttribute successfully
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, UnRegisterDisplayAttribute06, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    remoteMocker->SetRequestResult(ERR_NONE);
    std::vector<std::string> attributes = { "rotation" };
    sptr<IDisplayManagerAgent> agent = new DisplayManagerAgentDefault();
    DMError ret = proxy->UnregisterDisplayAttribute(attributes, agent);
    EXPECT_EQ(ret, DMError::DM_OK);
    EXPECT_TRUE(logMsg.find("SendRequest failed") == std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: UnRegisterDisplayAttribute07
 * @tc.desc: UnregisterDisplayAttribute with empty attributes
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, UnRegisterDisplayAttribute07, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    remoteMocker->SetRequestResult(ERR_NONE);
    std::vector<std::string> attributes = {};
    sptr<IDisplayManagerAgent> agent = new DisplayManagerAgentDefault();
    DMError ret = proxy->UnregisterDisplayAttribute(attributes, agent);
    EXPECT_EQ(ret, DMError::DM_OK);
    EXPECT_TRUE(logMsg.find("SendRequest failed") == std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetCurrentFoldCreaseRegion
 * @tc.desc: GetCurrentFoldCreaseRegion
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, GetCurrentFoldCreaseRegion, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MockMessageParcel::ClearAllErrorFlag();

    auto proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(nullptr);
    auto foldCreaseRegion = proxy->GetCurrentFoldCreaseRegion();
    EXPECT_TRUE(logMsg.find("remote is null") != std::string::npos);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    proxy = sptr<DisplayManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    foldCreaseRegion = proxy->GetCurrentFoldCreaseRegion();
    EXPECT_TRUE(logMsg.find("WriteInterfaceToken failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    foldCreaseRegion = proxy->GetCurrentFoldCreaseRegion();
    EXPECT_TRUE(logMsg.find("SendRequest failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    remoteMocker->SetRequestResult(ERR_NONE);
    foldCreaseRegion = proxy->GetCurrentFoldCreaseRegion();
    EXPECT_EQ(foldCreaseRegion, nullptr);

    logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: NotifyBootAnimationFinished
 * @tc.desc: NotifyBootAnimationFinished error and success paths
 * @tc.type: FUNC
 */
HWTEST_F(DisplayManagerLiteProxyMockTest, NotifyBootAnimationFinished, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    displayManagerLiteProxy_->NotifyBootAnimationFinished();
    EXPECT_TRUE(logMsg.find("WriteInterfaceToken failed") != std::string::npos);

    MockMessageParcel::ClearAllErrorFlag();
    displayManagerLiteProxy_->NotifyBootAnimationFinished();
    EXPECT_TRUE(logMsg.find("NotifyBootAnimationFinished async sent") != std::string::npos);

    logMsg.clear();
    LOG_SetCallback(nullptr);
}
} // namespace
} // namespace OHOS::Rosen