/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <message_option.h>
#include <message_parcel.h>
#include <common/rs_rect.h>
#include <transaction/rs_marshalling_helper.h>

#include "session_manager/include/scene_session_manager.h"
#include "session_manager/include/zidl/scene_session_manager_interface.h"
#include "window_manager_agent.h"
#include "zidl/screen_session_manager_stub.h"
#include "zidl/window_manager_agent_interface.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_logMsg = msg;
    }
    constexpr uint32_t SLEEP_TIME_IN_US = 100000; // 100ms
}
class ScreenSessionManagerStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<ScreenSessionManagerStub> stub_;
};

void ScreenSessionManagerStubTest::SetUpTestCase()
{
}

void ScreenSessionManagerStubTest::TearDownTestCase()
{
    usleep(SLEEP_TIME_IN_US);
}

void ScreenSessionManagerStubTest::SetUp()
{
    stub_ = new ScreenSessionManagerStub();
}

void ScreenSessionManagerStubTest::TearDown()
{
}

namespace {
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_DEFAULT_DISPLAY_INFO);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest02
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_REGISTER_DISPLAY_MANAGER_AGENT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name: OnRemoteRequest03
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest03, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_UNREGISTER_DISPLAY_MANAGER_AGENT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name: OnRemoteRequest04
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest04, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_WAKE_UP_BEGIN);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest05
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest05, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_WAKE_UP_END);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest06
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest06, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SUSPEND_BEGIN);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest07
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest07, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SUSPEND_END);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest08
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest08, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_DISPLAY_STATE);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest09
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest09, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_SCREEN_POWER_FOR_ALL);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest10
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest10, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_DISPLAY_STATE);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest11
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest11, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_EVENT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest12
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest12, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_SCREEN_POWER);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name: OnRemoteRequest13
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest13, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_DISPLAY_BY_ID);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest14
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest14, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_DISPLAY_BY_SCREEN);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest15
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest15, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_ALL_DISPLAYIDS);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest16
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest16, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_SCREEN_INFO_BY_ID);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest17
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest17, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_ALL_SCREEN_INFOS);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest18
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest18, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_GAMUTS);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest19
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest19, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_CREATE_VIRTUAL_SCREEN);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest20
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest20, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_DESTROY_VIRTUAL_SCREEN);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest21
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest21, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_PIXEL_RATIO);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest22
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest22, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_DESTROY_VIRTUAL_SCREEN);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest23
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest23, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_MIRROR);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest24
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest24, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_STOP_MIRROR);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest25
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest25, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_MAKE_EXPAND);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest26
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest26, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_STOP_EXPAND);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest27
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest27, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_SCREEN_GROUP_INFO_BY_ID);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest28
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest28, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_FROM_SCREEN_GROUP);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest29
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest29, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest30
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest30, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_SCREEN_ACTIVE_MODE);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest32
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest32, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_DISABLE_MIRROR);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest33
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest33, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteRemoteObject(nullptr);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_CLIENT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest34
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest34, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_SCREEN_PROPERTY);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest35
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest35, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_DISPLAY_NODE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest36
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest36, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    RRect bounds;
    RSMarshallingHelper::Marshalling(data, bounds);
    data.WriteFloat(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_ROTATION_PROPERTY);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest37
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest37, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_CURVED_SCREEN_COMPRESSION_AREA);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest38
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest38, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_PHY_SCREEN_PROPERTY);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest39
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest39, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_STATE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest40
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest40, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_COLOR_SPACE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest41
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest41, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_SPACE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest42
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest42, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_SET_HDR_FORMAT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest43
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest43, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_HDR_FORMAT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest44
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest44, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_HDR_FORMAT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest45
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest45, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_SET_PIXEL_FORMAT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest46
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest46, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_PIXEL_FORMAT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest47
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest47, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_TRANSFORM);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest48
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest48, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_SET_GAMUT_MAP);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest49
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest49, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_GAMUT_MAP);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest050
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest050, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_GAMUT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest051
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest051, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_COLOR_GAMUT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest052
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest052, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_DENSITY_IN_CURRENT_RESOLUTION);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest053
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest053, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_RESOLUTION);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest054
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest054, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_SPACE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest055
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest055, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_CANVAS_ROTATION);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest056
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest056, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_SCREEN_POWER);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, -1);
}

/**
 * @tc.name: OnRemoteRequest057
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest057, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_DEVICE_SCREEN_CONFIG);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest58
 * @tc.desc: normal function, TRANS_ID_SET_SCREENID_PRIVACY_STATE test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest58, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    DisplayId displayId = 0;
    data.WriteUint64(displayId);
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_SCREENID_PRIVACY_STATE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest59
 * @tc.desc: normal function, TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_LIST test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest59, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    DisplayId displayId = 0;
    data.WriteUint64(displayId);
    std::vector<std::string> privacyWindowList{"win0", "win1"};
    data.WriteStringVector(privacyWindowList);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_LIST);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest60
 * @tc.desc: normal function, TRANS_ID_SET_SCREEN_OFF_DELAY_TIME test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest60, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteInt32(2000);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_SCREEN_OFF_DELAY_TIME);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest61
 * @tc.desc: normal function, TRANS_ID_SET_VIRTUAL_SCREEN_BLACK_LIST test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest61, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId id = 1001;
    std::vector<uint64_t> windowIdList{10, 20, 30};
    data.WriteUint64(static_cast<uint64_t>(id));
    data.WriteUInt64Vector(windowIdList);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_BLACK_LIST);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest62
 * @tc.desc: normal function, TRANS_ID_NOTIFY_DISPLAY_HOOK_INFO test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest62, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteInt32(200);
    data.WriteBool(true);
    data.WriteUint32(1062);
    data.WriteUint32(1062);
    data.WriteFloat(1.5);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_HOOK_INFO);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest63
 * @tc.desc: normal function, TRANS_ID_RESET_ALL_FREEZE_STATUS test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest63, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_RESET_ALL_FREEZE_STATUS);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest64
 * @tc.desc: normal function, TRANS_ID_PROXY_FOR_FREEZE test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest64, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    int32_t size = 10;
    data.WriteInt32(size);
    for (int32_t i = 0; i < size; i++) {
        data.WriteInt32(i);
    }
    data.WriteBool(true);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_PROXY_FOR_FREEZE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest65
 * @tc.desc: normal function, TRANS_ID_DISABLE_POWEROFF_RENDER_CONTROL test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest65, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteUint64(1065);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_DISABLE_POWEROFF_RENDER_CONTROL);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest66
 * @tc.desc: normal function, TRANS_ID_SWITCH_USER test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest66, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SWITCH_USER);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest67
 * @tc.desc: normal function, TRANS_ID_SET_VIRTUAL_SCREEN_REFRESH_RATE test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest67, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteUint64(1067);
    data.WriteUint32(120);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_REFRESH_RATE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest68
 * @tc.desc: normal function, TRANS_ID_SET_VIRTUAL_SCREEN_FLAG test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest68, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteUint64(1068);
    data.WriteUint32(100);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_FLAG);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest69
 * @tc.desc: normal function, TRANS_ID_GET_VIRTUAL_SCREEN_FLAG test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest69, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteUint64(1069);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_VIRTUAL_SCREEN_FLAG);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest70
 * @tc.desc: normal function, TRANS_ID_NOTIFY_FOLD_TO_EXPAND_COMPLETION test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest70, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteBool(true);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_NOTIFY_FOLD_TO_EXPAND_COMPLETION);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest71
 * @tc.desc: normal function, TRANS_ID_GET_AVAILABLE_AREA test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest71, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteUint64(1071);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_AVAILABLE_AREA);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest72
 * @tc.desc: normal function, TRANS_ID_UPDATE_AVAILABLE_AREA test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest72, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteUint64(1072);
    data.WriteInt32(100);
    data.WriteInt32(200);
    data.WriteUint32(150);
    data.WriteInt32(250);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_UPDATE_AVAILABLE_AREA);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest73
 * @tc.desc: normal function, TRANS_ID_RESIZE_VIRTUAL_SCREEN test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest73, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteUint64(1073);
    data.WriteUint32(1080);
    data.WriteUint32(1440);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_RESIZE_VIRTUAL_SCREEN);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest74
 * @tc.desc: normal function, TRANS_ID_NOTIFY_DISPLAY_CHANGE_INFO test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest74, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    sptr<DisplayChangeInfo> info = new DisplayChangeInfo();
    info->action_ = ActionType::SINGLE_START;
    ScreenId toScreenId = SCREEN_ID_INVALID;
    info->toScreenId_ = toScreenId;
    ScreenId fromScreenId = SCREEN_ID_INVALID;
    info->fromScreenId_ = fromScreenId;
    std::string abilityName = "11";
    std::string bundleName = "22";
    info->abilityName_ = abilityName;
    info->bundleName_ = bundleName;
    info->Marshalling(data);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_CHANGE_INFO);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest75
 * @tc.desc: normal function, TRANS_ID_SCENE_BOARD_MAKE_UNIQUE_SCREEN test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest75, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    std::vector<ScreenId> uniqueScreenIds = {1008, 2008, 3008};
    data.WriteUint32(uniqueScreenIds.size());
    data.WriteUInt64Vector(uniqueScreenIds);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCENE_BOARD_MAKE_UNIQUE_SCREEN);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest76
 * @tc.desc: normal function, TRANS_ID_SCENE_BOARD_GET_CURRENT_FOLD_CREASE_REGION test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest76, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_CURRENT_FOLD_CREASE_REGION);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest77
 * @tc.desc: normal function, TRANS_ID_SCENE_BOARD_GET_FOLD_STATUS test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest77, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_STATUS);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest78
 * @tc.desc: normal function, TRANS_ID_SCENE_BOARD_IS_FOLDABLE test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest78, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCENE_BOARD_IS_FOLDABLE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest79
 * @tc.desc: normal function, TRANS_ID_SCENE_BOARD_GET_FOLD_DISPLAY_MODE test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest79, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_FOLD_DISPLAY_MODE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest80
 * @tc.desc: normal function, TRANS_ID_SCENE_BOARD_LOCK_FOLD_DISPLAY_STATUS test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest80, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCENE_BOARD_LOCK_FOLD_DISPLAY_STATUS);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest81
 * @tc.desc: normal function, TRANS_ID_SCENE_BOARD_SET_FOLD_DISPLAY_MODE test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest81, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCENE_BOARD_SET_FOLD_DISPLAY_MODE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest82
 * @tc.desc: normal function, TRANS_ID_DEVICE_IS_CAPTURE test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest82, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_DEVICE_IS_CAPTURE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest83
 * @tc.desc: normal function, TRANS_ID_SCENE_BOARD_DUMP_SPECIAL_SCREEN test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest83, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCENE_BOARD_DUMP_SPECIAL_SCREEN);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest84
 * @tc.desc: normal function, TRANS_ID_SCENE_BOARD_DUMP_ALL_SCREEN test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest84, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCENE_BOARD_DUMP_ALL_SCREEN);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest85
 * @tc.desc: normal function, TRANS_ID_HAS_IMMERSIVE_WINDOW test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest85, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0u);

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_HAS_IMMERSIVE_WINDOW);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest86
 * @tc.desc: normal function, TRANS_ID_CONVERT_SCREENID_TO_RSSCREENID test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest86, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteUint64(1086);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_CONVERT_SCREENID_TO_RSSCREENID);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest87
 * @tc.desc: normal function, TRANS_ID_HAS_PRIVATE_WINDOW test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest87, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteUint64(1087);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_HAS_PRIVATE_WINDOW);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest88
 * @tc.desc: normal function, TRANS_ID_GET_CUTOUT_INFO test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest88, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteUint64(1088);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_CUTOUT_INFO);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest89
 * @tc.desc: normal function, TRANS_ID_IS_SCREEN_ROTATION_LOCKED test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest89, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_IS_SCREEN_ROTATION_LOCKED);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest90
 * @tc.desc: normal function, TRANS_ID_SET_SCREEN_ROTATION_LOCKED_FROM_JS test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest90, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteBool(true);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_SCREEN_ROTATION_LOCKED_FROM_JS);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest91
 * @tc.desc: normal function, TRANS_ID_SET_SCREEN_ROTATION_LOCKED test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest91, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteBool(true);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_SCREEN_ROTATION_LOCKED);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest92
 * @tc.desc: normal function, TRANS_ID_SET_ORIENTATION test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest92, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteUint64(1092);
    data.WriteUint32(270);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_ORIENTATION);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest93
 * @tc.desc: normal function, TRANS_ID_SET_VIRTUAL_PIXEL_RATIO_SYSTEM test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest93, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteUint64(1093);
    data.WriteFloat(2.5);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_PIXEL_RATIO_SYSTEM);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest94
 * @tc.desc: normal function, TRANS_ID_GET_SNAPSHOT_BY_PICKER test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest94, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_SNAPSHOT_BY_PICKER);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest95
 * @tc.desc: normal function, TRANS_ID_SET_VIRTUAL_SCREEN_SCALE_MODE test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest95, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteUint64(1095);
    data.WriteUint32(25);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SCALE_MODE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest96
 * @tc.desc: normal function, TRANS_ID_SET_VIRTUAL_SCREEN_SURFACE test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest96, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_SURFACE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest97
 * @tc.desc: normal function, TRANS_ID_SET_SPECIFIED_SCREEN_POWER test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest97, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    data.WriteUint32(1097);
    data.WriteUint32(1);
    data.WriteUint32(12);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_SPECIFIED_SCREEN_POWER);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest98
 * @tc.desc: normal function, TRANS_ID_PROXY_FOR_FREEZE test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest98, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    int32_t size = 200;
    data.WriteInt32(size);
    for (int32_t i = 0; i < size; i++) {
        data.WriteInt32(i);
    }
    data.WriteBool(true);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_PROXY_FOR_FREEZE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest99
 * @tc.desc: normal function, invalid TRANS_ID test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest99, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(3500);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(res, 0);
}

/**
 * @tc.name: OnRemoteRequest100
 * @tc.desc: normal function, TRANS_ID_SET_VIRTUAL_SCREEN_BLACK_LIST test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest100, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId id = 1100;
    std::vector<uint64_t> windowIdList{};
    data.WriteUint64(static_cast<uint64_t>(id));
    data.WriteUInt64Vector(windowIdList);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_BLACK_LIST);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest101
 * @tc.desc: normal function, TRANS_ID_NOTIFY_DISPLAY_CHANGE_INFO test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest101, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    sptr<DisplayChangeInfo> info = new DisplayChangeInfo();
    info->Marshalling(data);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_NOTIFY_DISPLAY_CHANGE_INFO);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest102
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest102, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(1102);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_PHY_SCREEN_PROPERTY);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest103
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest103, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(1103);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_DISPLAY_NODE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest104
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest104, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(1104);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_SCREEN_PROPERTY);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest105
 * @tc.desc: normal function, TRANS_ID_SCENE_BOARD_MAKE_UNIQUE_SCREEN test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest105, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t size = 50;
    std::vector<ScreenId> uniqueScreenIds;
    for (uint32_t i = 0; i < size; i++) {
        uniqueScreenIds.push_back(i);
    }
    data.WriteUint32(size);
    data.WriteUInt64Vector(uniqueScreenIds);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCENE_BOARD_MAKE_UNIQUE_SCREEN);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest106
 * @tc.desc: normal function, TRANS_ID_SCENE_BOARD_MAKE_UNIQUE_SCREEN test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest106, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t size = 0;
    std::vector<ScreenId> uniqueScreenIds {};
    data.WriteUint32(size);
    data.WriteUInt64Vector(uniqueScreenIds);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCENE_BOARD_MAKE_UNIQUE_SCREEN);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest107
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest107, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(1107);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_COLOR_SPACE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest108
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest108, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(1108);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_COLOR_SPACE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest109
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest109, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(1109);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_HDR_FORMAT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest110
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest110, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(1100);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTED_HDR_FORMAT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest111
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest111, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(1111);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_PIXEL_FORMAT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest112
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest112, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(1112);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_GAMUT_MAP);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest113
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest113, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(1113);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_COLOR_GAMUT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest114
 * @tc.desc: normal function, TRANS_ID_GET_ALL_PHYSICAL_DISPLAY_RESOLUTION test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest114, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_ALL_PHYSICAL_DISPLAY_RESOLUTION);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest115
 * @tc.desc: normal function, TRANS_ID_GET_SCREEN_POWER
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest115, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1115;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_SCREEN_POWER);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest116
 * @tc.desc: normal function, TRANS_ID_SET_VIRTUAL_SCREEN_CANVAS_ROTATION
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest116, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1116;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_CANVAS_ROTATION);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest117
 * @tc.desc: normal function, TRANS_ID_SET_RESOLUTION
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest117, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1117;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    uint32_t width = 1080;
    data.WriteUint32(width);
    uint32_t height = 2560;
    data.WriteUint32(height);
    float virtualPixelRatio = 64;
    data.WriteFloat(virtualPixelRatio);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_RESOLUTION);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest118
 * @tc.desc: normal function, TRANS_ID_GET_DENSITY_IN_CURRENT_RESOLUTION
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest118, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1118;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_DENSITY_IN_CURRENT_RESOLUTION);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest119
 * @tc.desc: normal function, TRANS_ID_SCREEN_SET_COLOR_GAMUT
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest119, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1119;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    int32_t colorGamutIdx = 64;
    data.WriteInt32(colorGamutIdx);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_GAMUT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest120
 * @tc.desc: normal function, TRANS_ID_SCREEN_SET_GAMUT_MAP
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest120, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1119;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    uint32_t colorGamutIdx = 64;
    data.WriteUint32(colorGamutIdx);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_SET_GAMUT_MAP);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest121
 * @tc.desc: normal function, TRANS_ID_SCREEN_SET_COLOR_TRANSFORM
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest121, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1121;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_TRANSFORM);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest122
 * @tc.desc: normal function, TRANS_ID_SCREEN_SET_PIXEL_FORMAT
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest122, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1122;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    int32_t pixelFormat = 64;
    data.WriteInt32(pixelFormat);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_SET_PIXEL_FORMAT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest123
 * @tc.desc: normal function, TRANS_ID_SCREEN_SET_HDR_FORMAT
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest123, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1123;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    int32_t modeIdx = 120;
    data.WriteInt32(modeIdx);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_SET_HDR_FORMAT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest124
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest124, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1123;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    int32_t colorSpace = 32;
    data.WriteInt32(colorSpace);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_SPACE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest125
 * @tc.desc: TRANS_ID_SCENE_BOARD_SET_DISPLAY_SCALE test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest125, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ScreenId screenId = 0;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float pivotX = 0.5f;
    float pivotY = 0.5f;
    ASSERT_TRUE(data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor()) &&
                data.WriteUint64(static_cast<uint64_t>(screenId)) && data.WriteFloat(scaleX) &&
                data.WriteFloat(scaleY) && data.WriteFloat(pivotX) && data.WriteFloat(pivotY));
    uint32_t code =
        static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCENE_BOARD_SET_DISPLAY_SCALE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest126
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest126, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_INTERNAL_SCREEN_ID);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest127
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest127, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId screenId = 0;
    data.WriteUint64(screenId);
    ScreenPowerState state = ScreenPowerState{0};
    data.WriteUint32(static_cast<uint32_t>(state));
    PowerStateChangeReason reason = PowerStateChangeReason{0};
    data.WriteUint32(static_cast<uint32_t>(reason));
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_SCREEN_POWER_BY_ID);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest128
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest128, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_TRY_TO_CANCEL_SCREEN_OFF);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest129
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest129, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId mainScreenId = 0;
    data.WriteUint64(static_cast<uint64_t>(mainScreenId));
    ScreenId secondaryScreenId = 1;
    data.WriteUint64(static_cast<uint64_t>(secondaryScreenId));
    MultiScreenMode screenMode = MultiScreenMode::SCREEN_EXTEND;
    data.WriteUint32(static_cast<uint32_t>(screenMode));
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_MULTI_SCREEN_MODE_SWITCH);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest130
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest130, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    data.WriteUint32(0);
    data.WriteUint32(0);
    data.WriteUint64(1);
    data.WriteUint32(0);
    data.WriteUint32(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_MULTI_SCREEN_POSITION);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest131
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest131, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId screenId = 100;
    data.WriteUint64(static_cast<uint64_t>(screenId));
    data.WriteInt32(static_cast<int32_t>(0));
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_STATUS);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest132
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest132, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId id = 100;
    data.WriteUint64(static_cast<uint64_t>(id));
    uint32_t refreshRate = 12;
    data.WriteUint32(refreshRate);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_MAX_REFRESHRATE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest133
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest133, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId id = 0;
    data.WriteUint64(static_cast<uint64_t>(id));
    data.WriteBool(true);
    data.WriteBool(true);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_CAPTURE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest13301
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest13301, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId id = 0;
    data.WriteUint64(static_cast<uint64_t>(id));
    data.WriteBool(true);
    data.WriteBool(true);
    data.WriteBool(true);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_CAPTURE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_TRUE(g_logMsg.find("Read node blackWindowIdList failed") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: OnRemoteRequest134
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest134, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PRIMARY_DISPLAY_INFO);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest135
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest135, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    data.WriteFloat(0);
    data.WriteFloat(0);
    data.WriteFloat(0);
    data.WriteUint32(2);
    RRect bounds;
    RSMarshallingHelper::Marshalling(data, bounds);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_UPDATE_SCREEN_DIRECTION_INFO);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}
/**
 * @tc.name: OnRemoteRequest136
 * @tc.desc: normal function, TRANS_ID_GET_SCREEN_POWER
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest136, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1115;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));

    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_POWER);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest137
 * @tc.desc: normal function, TRANS_ID_SET_VIRTUAL_SCREEN_CANVAS_ROTATION
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest137, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1116;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_SCREEN_CANVAS_ROTATION);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest138
 * @tc.desc: normal function, TRANS_ID_SET_RESOLUTION
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest138, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1117;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    uint32_t width = 1080;
    data.WriteUint32(width);
    uint32_t height = 2560;
    data.WriteUint32(height);
    float virtualPixelRatio = 64;
    data.WriteFloat(virtualPixelRatio);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_RESOLUTION);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest139
 * @tc.desc: normal function, TRANS_ID_GET_DENSITY_IN_CURRENT_RESOLUTION
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest139, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1118;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DENSITY_IN_CURRENT_RESOLUTION);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest140
 * @tc.desc: normal function, TRANS_ID_SCREEN_SET_COLOR_GAMUT
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest140, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1119;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    int32_t colorGamutIdx = 64;
    data.WriteInt32(colorGamutIdx);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_GAMUT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest141
 * @tc.desc: normal function, TRANS_ID_SCREEN_SET_GAMUT_MAP
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest141, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1119;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    uint32_t colorGamutIdx = 64;
    data.WriteUint32(colorGamutIdx);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_GAMUT_MAP);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest142
 * @tc.desc: normal function, TRANS_ID_SCREEN_SET_COLOR_TRANSFORM
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest142, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1121;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_TRANSFORM);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest143
 * @tc.desc: normal function, TRANS_ID_SCREEN_SET_PIXEL_FORMAT
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest143, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1122;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    int32_t pixelFormat = 64;
    data.WriteInt32(pixelFormat);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_PIXEL_FORMAT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest144
 * @tc.desc: normal function, TRANS_ID_SCREEN_SET_HDR_FORMAT
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest144, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1123;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    int32_t modeIdx = 120;
    data.WriteInt32(modeIdx);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_HDR_FORMAT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest145
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest145, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1123;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    int32_t colorSpace = 32;
    data.WriteInt32(colorSpace);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SCREEN_SET_COLOR_SPACE);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest146
 * @tc.desc: normal function, TRANS_ID_SCENE_BOARD_GET_SUPER_FOLD_STATUS test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest146, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCENE_BOARD_GET_SUPER_FOLD_STATUS);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest147
 * @tc.desc: normal function, TRANS_ID_SCENE_BOARD_GET_SUPER_FOLD_STATUS test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest147, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    const std::vector<ScreenId> screenIds = {1001, 1002};
    bool isEnable = true;
    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUInt64Vector(screenIds);
    data.WriteBool(isEnable);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_SKIP_PROTECTED_WINDOW);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest148
 * @tc.desc: normal function, TRANS_ID_GET_DISPLAY_CAPABILITY test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest148, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_CAPABILITY);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest149
 * @tc.desc: normal function, TRANS_ID_SET_SYSTEM_KEYBOARD_STATUS test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest149, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    bool isTpKeyboardOn = true;
    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteBool(isTpKeyboardOn);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SYSTEM_KEYBOARD_STATUS);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest150
 * @tc.desc: normal function, TRANS_ID_SET_VIRTUAL_DISPLAY_MUTE_FLAG test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest150, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    ScreenId testScreenId = 1123;
    data.WriteUint64(static_cast<uint64_t>(testScreenId));
    bool muteFlag = false;
    data.WriteBool(muteFlag);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_VIRTUAL_DISPLAY_MUTE_FLAG);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest151
 * @tc.desc: normal function, TRANS_ID_ADD_VIRTUAL_SCREEN_BLACK_LIST test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest151, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(1u);
    data.WriteInt32(1);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_ADD_VIRTUAL_SCREEN_BLOCK_LIST);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}


/**
 * @tc.name: OnRemoteRequest152
 * @tc.desc: normal function, TRANS_ID_REMOVE_VIRTUAL_SCREEN_BLACK_LIST test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest152, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(1u);
    data.WriteInt32(1);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_BLOCK_LIST);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest153
 * @tc.desc: normal function, TRANS_ID_SET_FOLD_STATUS_EXPAND_AND_LOCKED test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest153, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(1u);
    data.WriteInt32(1);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_FOLD_STATUS_EXPAND_AND_LOCKED);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest154
 * @tc.desc: normal function, TRANS_ID_GET_SCREEN_AREA_OF_DISPLAY_AREA test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest154, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(1u);
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteUint32(1u);
    data.WriteUint32(1u);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_SCREEN_AREA_OF_DISPLAY_AREA);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest40
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest155, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_SCREEN_BRIGHTNESS_INFO);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest155_1
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest155_1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_SUPPORTS_INPUT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest155_2
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest155_2, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_SET_SUPPORTS_INPUT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest155_3
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest155_3, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_GET_ROUNDED_CORNER);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest155_4
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest155_4, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_SCREEN_GET_BUNDLE_NAME);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest156
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest156, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_ADD_VIRTUAL_SCREEN_WHITE_LIST);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest157
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, OnRemoteRequest157, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(0);
    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_WHITE_LIST);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: ProcSetPrimaryDisplaySystemDpi
 * @tc.desc: normal function, TRANS_ID_SET_PRIMARY_DISPLAY_SYSTEM_DPI test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, ProcSetPrimaryDisplaySystemDpi, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteFloat(2.2f);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_PRIMARY_DISPLAY_SYSTEM_DPI);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: GetIPCPriority
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, GetIPCPriorityTest, TestSize.Level1)
{
    IPCPriority IPCPriority = stub_->GetIPCPriority(0);
    EXPECT_EQ(IPCPriority, IPCPriority::VIP);
    IPCPriority = stub_->GetIPCPriority(1000);
    EXPECT_EQ(IPCPriority, IPCPriority::LOW);
}

/**
 * @tc.name: GetPhysicalScreenIds
 * @tc.desc: normal function, GetPhysicalScreenIds test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, GetPhysicalScreenIds, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::vector<ScreenId> screenIds;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_PHYSICAL_SCREEN_IDS);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: SetScreenPrivacyWindowTagSwitch
 * @tc.desc: normal function, TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_TAG_SWITCH test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, SetScreenPrivacyWindowTagSwitch, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ScreenId mainScreenId = 0;
    std::vector<std::string> privacyWindowTag{"test1", "test2"};
    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(mainScreenId);
    data.WriteStringVector(privacyWindowTag);
    data.WriteBool(true);
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SET_SCREEN_PRIVACY_WINDOW_TAG_SWITCH);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: NotifySwitchUserAnimationFinish
 * @tc.desc: normal function, TRANS_ID_NOTIFY_SWITCH_USER_ANIMATION_FINISH test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, NotifySwitchUserAnimationFinish, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::vector<ScreenId> screenIds;

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_SWITCH_USER_ANIMATION_FINISH);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: NotifyIsFullScreenInForceSplitMode
 * @tc.desc: normal function, TRANS_ID_NOTIFY_IS_FULL_SCREEN_IN_FORCE_SPLIT test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, NotifyIsFullScreenInForceSplitMode, TestSize.Level3)
{
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_IS_FULL_SCREEN_IN_FORCE_SPLIT);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 1);
    }
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_NOTIFY_IS_FULL_SCREEN_IN_FORCE_SPLIT);
        data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
        data.WriteInt32(0);
        data.WriteBool(true);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, ERR_NONE);
    }
}

/**
 * @tc.name: SyncScreenPowerState
 * @tc.desc: normal function, TRANS_ID_SYNC_SCREEN_POWER_STATE test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, SyncScreenPowerState, TestSize.Level3)
{
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SYNC_SCREEN_POWER_STATE);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, ERR_TRANSACTION_FAILED);
    }
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_SYNC_SCREEN_POWER_STATE);
        data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
        data.WriteUint32(0);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, ERR_NONE);
    }
}

/**
 * @tc.name: ProcGetDisplaySnapshotWithOption01
 * @tc.desc: normal function, ProcGetDisplaySnapshotWithOption test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, ProcGetDisplaySnapshotWithOption01, TestSize.Level1)
{
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT_WITH_OPTION);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, ERR_TRANSACTION_FAILED);
    }
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT_WITH_OPTION);
        data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
        data.WriteUint64(0);
        data.WriteBool(false);
        data.WriteBool(false);
        data.WriteBool(false);
        data.WriteFloat(0.5f);
        data.WriteFloat(0.5f);
        data.WriteInt32(200);
        data.WriteInt32(200);
        data.WriteUint32(200);
        data.WriteUint32(200);
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, ERR_NONE);
    }
}

/**
 * @tc.name: ProcGetDisplaySnapshotWithOption02
 * @tc.desc: normal function, ProcGetDisplaySnapshotWithOption test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, ProcGetDisplaySnapshotWithOption02, TestSize.Level1)
{
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT_WITH_OPTION);
        data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        data.WriteUint64(0);
        data.WriteBool(false);
        data.WriteBool(false);
        data.WriteBool(false);
        stub_->ProcGetDisplaySnapshotWithOption(data, reply);
        EXPECT_EQ(res, 0);
    }
}

/**
 * @tc.name: ProcGetDisplaySnapshotWithOption03
 * @tc.desc: normal function, ProcGetDisplaySnapshotWithOption test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, ProcGetDisplaySnapshotWithOption03, TestSize.Level1)
{
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT_WITH_OPTION);
        data.WriteUint64(0);
        data.WriteBool(false);
        data.WriteBool(false);
        data.WriteBool(false);
        std::vector<uint64_t> windowIdList{10, 20, 30};
        data.WriteUInt64Vector(windowIdList);
        data.WriteFloat(0.5f);
        data.WriteFloat(0.5f);
        stub_->ProcGetDisplaySnapshotWithOption(data, reply);
        data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }
}

/**
 * @tc.name: ProcGetDisplaySnapshotWithOption04
 * @tc.desc: normal function, ProcGetDisplaySnapshotWithOption test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, ProcGetDisplaySnapshotWithOption04, TestSize.Level1)
{
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT_WITH_OPTION);
        data.WriteUint64(0);
        data.WriteBool(false);
        data.WriteBool(false);
        data.WriteBool(false);
        std::vector<uint64_t> windowIdList{10, 20, 30};
        data.WriteUInt64Vector(windowIdList);
        data.WriteFloat(0.5f);
        data.WriteFloat(0.5f);
        data.WriteInt32(200);
        data.WriteInt32(200);
        stub_->ProcGetDisplaySnapshotWithOption(data, reply);
        data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }
}

/**
 * @tc.name: ProcGetDisplaySnapshotWithOption05
 * @tc.desc: normal function, ProcGetDisplaySnapshotWithOption test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, ProcGetDisplaySnapshotWithOption05, TestSize.Level1)
{
    {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_GET_DISPLAY_SNAPSHOT_WITH_OPTION);
        data.WriteUint64(0);
        data.WriteBool(false);
        data.WriteBool(false);
        data.WriteBool(false);
        std::vector<uint64_t> windowIdList{10, 20, 30};
        data.WriteUInt64Vector(windowIdList);
        data.WriteFloat(0.5f);
        data.WriteFloat(0.5f);
        data.WriteInt32(200);
        data.WriteInt32(200);
        data.WriteUint32(200);
        stub_->ProcGetDisplaySnapshotWithOption(data, reply);
        data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
        int res = stub_->OnRemoteRequest(code, data, reply, option);
        EXPECT_EQ(res, 0);
    }
}

/**
 * @tc.name: IsOnboardDisplay
 * @tc.desc: IsOnboardDisplay
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubTest, IsOnboardDisplay, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(DisplayManagerMessage::TRANS_ID_IS_ON_BOARD_DISPLAY);
    DisplayId displayId = 0;

    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    data.WriteInterfaceToken(ScreenSessionManagerStub::GetDescriptor());
    data.WriteUint64(displayId);
    stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_TRUE(g_logMsg.find("write res failed") == std::string::npos);
    LOG_SetCallback(nullptr);
}
}
}
}