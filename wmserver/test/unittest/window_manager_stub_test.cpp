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
#include "window_manager_stub_impl.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class WindowManagerStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<WindowManagerStub> stub_;
};

void WindowManagerStubTest::SetUpTestCase()
{
}

void WindowManagerStubTest::TearDownTestCase()
{
}

void WindowManagerStubTest::SetUp()
{
    stub_ = new WindowManagerStubImpl();
}

void WindowManagerStubTest::TearDown()
{
}

namespace {
/**
 * @tc.name: OnRemoteRequest01
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(u"error.GetDescriptor");

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_CREATE_WINDOW);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_TRANSACTION_FAILED));
}

/**
 * @tc.name: OnRemoteRequest02
 * @tc.desc: test TRANS_ID_REMOVE_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_REMOVE_WINDOW);

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteUint32(1);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteBool(false);
    res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteUint32(1);
    data.WriteBool(false);
    res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest03
 * @tc.desc: test TRANS_ID_REQUEST_FOCUS success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest03, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteUint32(1);

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_REQUEST_FOCUS);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest04
 * @tc.desc: test TRANS_ID_GET_AVOID_AREA success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest04, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteUint32(1);
    data.WriteUint32(static_cast<uint32_t>(AvoidAreaType::TYPE_CUTOUT));

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_GET_AVOID_AREA);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest05
 * @tc.desc: test TRANS_ID_UNREGISTER_WINDOW_MANAGER_AGENT success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest05, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CAMERA_FLOAT));
    sptr<IWindowManagerAgent> focusChangedListenerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(focusChangedListenerAgent->AsObject());

    uint32_t code =
        static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_UNREGISTER_WINDOW_MANAGER_AGENT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest06
 * @tc.desc: test TRANS_ID_NOTIFY_READY_MOVE_OR_DRAG success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest06, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteUint32(1);
    sptr<WindowProperty> windowProperty = new WindowProperty();
    data.WriteParcelable(windowProperty.GetRefPtr());
    sptr<MoveDragProperty> moveDragProperty = new MoveDragProperty();
    data.WriteParcelable(moveDragProperty.GetRefPtr());

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_NOTIFY_READY_MOVE_OR_DRAG);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest07
 * @tc.desc: test TRANS_ID_PROCESS_POINT_DOWN success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest07, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteUint32(1);
    data.WriteBool(false);

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_PROCESS_POINT_DOWN);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest08
 * @tc.desc: test TRANS_ID_PROCESS_POINT_UP success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest08, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteUint32(1);

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_PROCESS_POINT_UP);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest09
 * @tc.desc: test TRANS_ID_GET_TOP_WINDOW_ID success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest09, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteUint32(1);

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_GET_TOP_WINDOW_ID);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest10
 * @tc.desc: test TRANS_ID_MINIMIZE_ALL_APP_WINDOWS success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest10, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteUint64(0);
    data.WriteInt32(0);

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_MINIMIZE_ALL_APP_WINDOWS);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest11
 * @tc.desc: test TRANS_ID_TOGGLE_SHOWN_STATE_FOR_ALL_APP_WINDOWS success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest11, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());

    uint32_t code =
        static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_TOGGLE_SHOWN_STATE_FOR_ALL_APP_WINDOWS);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest12
 * @tc.desc: test TRANS_ID_UPDATE_LAYOUT_MODE success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest12, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(WindowLayoutMode::CASCADE));

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_UPDATE_LAYOUT_MODE);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest13
 * @tc.desc: test TRANS_ID_UPDATE_PROPERTY success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest13, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteUint64(static_cast<uint64_t>(PropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG));
    sptr<WindowProperty> windowProperty = new WindowProperty();
    windowProperty->Write(data, PropertyChangeAction::ACTION_UPDATE_ANIMATION_FLAG);

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_UPDATE_PROPERTY);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest14
 * @tc.desc: test TRANS_ID_ANIMATION_SET_CONTROLLER success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest14, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_ANIMATION_SET_CONTROLLER);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: OnRemoteRequest15
 * @tc.desc: test TRANS_ID_NOTIFY_WINDOW_TRANSITION success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest15, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    sptr<WindowTransitionInfo> from = new WindowTransitionInfo();
    sptr<WindowTransitionInfo> to = new WindowTransitionInfo();
    data.WriteParcelable(from);
    data.WriteParcelable(to);
    data.WriteBool(false);

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_NOTIFY_WINDOW_TRANSITION);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest16
 * @tc.desc: test TRANS_ID_GET_FULLSCREEN_AND_SPLIT_HOT_ZONE success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest16, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    DisplayId displayId = 0;
    data.WriteUint64(displayId);

    uint32_t code =
        static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_GET_FULLSCREEN_AND_SPLIT_HOT_ZONE);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest17
 * @tc.desc: test TRANS_ID_GET_ANIMATION_CALLBACK success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest17, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    std::vector<uint32_t> windowIds;
    windowIds.emplace_back(1);
    windowIds.emplace_back(10);
    bool isAnimated = false;
    data.WriteUInt32Vector(windowIds);
    data.WriteBool(isAnimated);

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_GET_ANIMATION_CALLBACK);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest18
 * @tc.desc: test TRANS_ID_UPDATE_RS_TREE success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest18, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    uint32_t windowId = 1;
    bool isAdd = false;
    data.WriteUint32(windowId);
    data.WriteBool(isAdd);

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_UPDATE_RS_TREE);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest19
 * @tc.desc: test TRANS_ID_CREATE_WINDOW success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest19, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    sptr<WindowOption> windowOption = new WindowOption();
    sptr<WindowImpl> windowImpl = new WindowImpl(windowOption);
    sptr<IWindow> window = new WindowAgent(windowImpl);
    sptr<WindowProperty> property = nullptr;
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "SurfaceNode";
    std::shared_ptr<RSSurfaceNode> surNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    sptr<IRemoteObject> token;

    data.WriteRemoteObject(window->AsObject());
    data.WriteParcelable(property.GetRefPtr());
    surNode->Marshalling(data);
    data.WriteRemoteObject(token);

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_CREATE_WINDOW);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest20
 * @tc.desc: test TRANS_ID_CREATE_WINDOW success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest20, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    sptr<WindowOption> windowOption = new WindowOption();
    sptr<WindowImpl> windowImpl = new WindowImpl(windowOption);
    sptr<IWindow> window = new WindowAgent(windowImpl);
    sptr<WindowProperty> property = new WindowProperty();
    property->SetTokenState(false);
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    surfaceNodeConfig.SurfaceNodeName = "SurfaceNode";
    std::shared_ptr<RSSurfaceNode> surNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    sptr<IRemoteObject> token;

    data.WriteRemoteObject(window->AsObject());
    data.WriteParcelable(property.GetRefPtr());
    surNode->Marshalling(data);
    data.WriteRemoteObject(token);

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_CREATE_WINDOW);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest21
 * @tc.desc: test TRANS_ID_RAISE_WINDOW_Z_ORDER success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest21, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteUint32(100);

    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_RAISE_WINDOW_Z_ORDER);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest22
 * @tc.desc: test TRANS_ID_RAISE_WINDOW_Z_ORDER success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest22, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteBool(true);
    uint32_t code = static_cast<uint32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_GESTURE_NAVIGATION_ENABLED);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest23
 * @tc.desc: test TRANS_ID_GET_UNRELIABLE_WINDOW_INFO_ID success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest23, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteInt32(0);
    uint32_t code = static_cast<uint32_t>(
        IWindowManager::WindowManagerMessage::TRANS_ID_GET_UNRELIABLE_WINDOW_INFO_ID);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest24
 * @tc.desc: test TRANS_ID_GET_SNAPSHOT success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest24, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    uint32_t writeValue = 0;
    data.WriteInt32(writeValue);
    uint32_t code = static_cast<uint32_t>(
        IWindowManager::WindowManagerMessage::TRANS_ID_GET_SNAPSHOT);
    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest25
 * @tc.desc: test TRANS_ID_NOTIFY_SCREEN_SHOT_EVENT success
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerStubTest, OnRemoteRequest25, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowManagerStub::GetDescriptor());
    data.WriteInt32(static_cast<int32_t>(ScreenshotEventType::SCROLL_SHOT_START));

    int32_t code = static_cast<int32_t>(IWindowManager::WindowManagerMessage::TRANS_ID_NOTIFY_SCREEN_SHOT_EVENT);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}
}
}
}
