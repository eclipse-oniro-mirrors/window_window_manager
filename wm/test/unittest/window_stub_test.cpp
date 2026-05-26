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
#include "window_agent.h"
#include "window_stub.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class WindowStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<WindowStub> windowStub_;
};

void WindowStubTest::SetUpTestCase() {}

void WindowStubTest::TearDownTestCase()
{
}

void WindowStubTest::SetUp()
{
    sptr<WindowOption> option = new WindowOption();
    sptr<WindowImpl> window = new WindowImpl(option);
    windowStub_ = new WindowAgent(window);
}

void WindowStubTest::TearDown() {}

namespace {
/**
 * @tc.name: OnRemoteRequest01
 * @tc.desc: test InterfaceToken check failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(u"error.GetDescriptor");

    data.WriteInt32(0);
    data.WriteInt32(0);
    data.WriteUint32(100);
    data.WriteUint32(100);

    data.WriteBool(false);

    data.WriteUint32(static_cast<uint32_t>(WindowSizeChangeReason::DRAG_START));

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_WINDOW_RECT);

    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_TRANSACTION_FAILED));
}

/**
 * @tc.name: OnRemoteRequest02
 * @tc.desc: test TRANS_ID_UPDATE_WINDOW_RECT
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    data.WriteInt32(0);
    data.WriteInt32(0);
    data.WriteUint32(100);
    data.WriteUint32(100);

    data.WriteBool(false);

    data.WriteUint32(static_cast<uint32_t>(WindowSizeChangeReason::DRAG_START));

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_WINDOW_RECT);

    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: OnRemoteRequest03
 * @tc.desc: test TRANS_ID_UPDATE_AVOID_AREA success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest03, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    sptr<AvoidArea> avoidArea = new AvoidArea();
    data.WriteStrongParcelable(avoidArea);

    data.WriteUint32(static_cast<uint32_t>(AvoidAreaType::TYPE_SYSTEM));

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_AVOID_AREA);

    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest04
 * @tc.desc: test TRANS_ID_UPDATE_AVOID_AREA success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest04, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    sptr<AvoidArea> avoidArea = new AvoidArea();
    data.WriteStrongParcelable(avoidArea);

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_AVOID_AREA);

    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: OnRemoteRequest05
 * @tc.desc: test TRANS_ID_DUMP_INFO success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest05, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    std::vector<std::string> params;
    params.push_back("-a");
    data.WriteStringVector(params);

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_DUMP_INFO);

    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest06
 * @tc.desc: test TRANS_ID_DUMP_INFO failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest06, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowStub::GetDescriptor());
    data.WriteRawData(nullptr, 0);

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_DUMP_INFO);

    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest07
 * @tc.desc: test TRANS_ID_NOTIFY_CLIENT_POINT_UP success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest07, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->WriteToParcel(data);

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_CLIENT_POINT_UP);

    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest08
 * @tc.desc: test TRANS_ID_NOTIFY_CLIENT_POINT_UP success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest08, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_CLIENT_POINT_UP);

    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: OnRemoteRequest09
 * @tc.desc: test TRANS_ID_NOTIFY_FOREGROUND_INTERACTIVE_STATUS success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest09, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_FOREGROUND_INTERACTIVE_STATUS);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest10
 * @tc.desc: test TRANS_ID_NOTIFY_FOREGROUND_INTERACTIVE_STATUS success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest10, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    uint32_t code = 0;

    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_TRANSACTION_FAILED));
}

/**
 * @tc.name: OnRemoteRequest11
 * @tc.desc: test TRANS_ID_NOTIFY_FOREGROUND_INTERACTIVE_STATUS success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest11, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_WINDOW_RECT);
    data.WriteBool(false);
    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_WINDOW_MODE);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_MODE_SUPPORT_INFO);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_FOCUS_STATUS);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_AVOID_AREA);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    uint32_t type = 1;
    data.ReadUint32(type);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_WINDOW_STATE);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_DRAG_EVENT);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_DISPLAY_ID);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_OCCUPIED_AREA);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_OCCUPIED_AREA_AND_RECT);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_ACTIVE_STATUS);

    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(res, 10);
}

/**
 * @tc.name: OnRemoteRequest12
 * @tc.desc: test TRANS_ID_NOTIFY_FOREGROUND_INTERACTIVE_STATUS success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest12, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_WINDOW_RECT);
    data.WriteBool(false);
    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_GET_WINDOW_PROPERTY);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_OUTSIDE_PRESSED);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_SCREEN_SHOT);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_DESTROY);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_FOREGROUND);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_BACKGROUND);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_DUMP_INFO);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_CLIENT_POINT_UP);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_ZOOM_TRANSFORM);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_RESTORE_SPLIT_WINDOW_MODE);
    windowStub_->OnRemoteRequest(code, data, reply, option);
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_FOREGROUND_INTERACTIVE_STATUS);

    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(res, 10);
}

/**
 * @tc.name: OnRemoteRequest13
 * @tc.desc: test TRANS_ID_UPDATE_WINDOW_RECT success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest13, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_WINDOW_RECT);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: OnRemoteRequest14
 * @tc.desc: test TRANS_ID_UPDATE_WINDOW_MODE read windowMode failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest14, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_WINDOW_MODE);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));

    data.WriteInterfaceToken(WindowStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(WindowMode::WINDOW_MODE_FB) + 1);
    res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: UpdateWindowMode01
 * @tc.desc: test TRANS_ID_UPDATE_WINDOW_MODE success with WindowModeInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, UpdateWindowMode01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_WINDOW_MODE);

    // SPLIT_PRIMARY with splitStyle and splitIndex
    data.WriteInterfaceToken(WindowStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(WindowMode::WINDOW_MODE_SPLIT_PRIMARY));
    data.WriteUint32(static_cast<uint32_t>(SplitStyle::TWO_WINDOW_HORIZONTAL));
    data.WriteInt32(SPLIT_INDEX_PRIMARY);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));

    // SPLIT_SECONDARY with vertical style
    data.WriteInterfaceToken(WindowStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(WindowMode::WINDOW_MODE_SPLIT_SECONDARY));
    data.WriteUint32(static_cast<uint32_t>(SplitStyle::TWO_WINDOW_VERTICAL));
    data.WriteInt32(SPLIT_INDEX_SECONDARY);
    res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));

    // boundary: UNDEFINED (lower bound)
    data.WriteInterfaceToken(WindowStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(WindowMode::WINDOW_MODE_UNDEFINED));
    data.WriteUint32(static_cast<uint32_t>(SplitStyle::TWO_WINDOW_HORIZONTAL));
    data.WriteInt32(SPLIT_INDEX_PRIMARY);
    res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));

    // boundary: PIP (upper bound of range check)
    data.WriteInterfaceToken(WindowStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(WindowMode::WINDOW_MODE_PIP));
    data.WriteUint32(static_cast<uint32_t>(SplitStyle::TWO_WINDOW_HORIZONTAL));
    data.WriteInt32(SPLIT_INDEX_PRIMARY);
    res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: UpdateWindowMode02
 * @tc.desc: test TRANS_ID_UPDATE_WINDOW_MODE invalid windowMode range
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, UpdateWindowMode02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_WINDOW_MODE);

    // WINDOW_MODE_SPLIT (105) exceeds PIP (103), rejected by range check
    data.WriteInterfaceToken(WindowStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(WindowMode::WINDOW_MODE_SPLIT));
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: UpdateWindowMode03
 * @tc.desc: test TRANS_ID_UPDATE_WINDOW_MODE read splitStyle/splitIndex failed
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, UpdateWindowMode03, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_WINDOW_MODE);

    // valid windowMode, missing splitStyle
    data.WriteInterfaceToken(WindowStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(WindowMode::WINDOW_MODE_FULLSCREEN));
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));

    // valid windowMode + splitStyle, missing splitIndex
    data.WriteInterfaceToken(WindowStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(WindowMode::WINDOW_MODE_FULLSCREEN));
    data.WriteUint32(static_cast<uint32_t>(SplitStyle::TWO_WINDOW_HORIZONTAL));
    res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: OnRemoteRequest15
 * @tc.desc: test TRANS_ID_UPDATE_MODE_SUPPORT_INFO success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest15, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_MODE_SUPPORT_INFO);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: OnRemoteRequest16
 * @tc.desc: test TRANS_ID_UPDATE_FOCUS_STATUS success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest16, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_FOCUS_STATUS);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest17
 * @tc.desc: test TRANS_ID_UPDATE_AVOID_AREA success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest17, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_AVOID_AREA);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: OnRemoteRequest18
 * @tc.desc: test TRANS_ID_UPDATE_WINDOW_STATE success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest18, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());
    data.WriteUint32(1);

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_WINDOW_STATE);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_NONE));
}

/**
 * @tc.name: OnRemoteRequest19
 * @tc.desc: test TRANS_ID_UPDATE_DRAG_EVENT success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest19, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());
    data.WriteInt32(0);
    data.WriteInt32(0);
    data.WriteUint32(1);

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_DRAG_EVENT);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest20
 * @tc.desc: test TRANS_ID_UPDATE_DISPLAY_ID success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest20, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_DISPLAY_ID);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: OnRemoteRequest21
 * @tc.desc: test TRANS_ID_UPDATE_OCCUPIED_AREA success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest21, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_OCCUPIED_AREA);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: OnRemoteRequest22
 * @tc.desc: test TRANS_ID_UPDATE_OCCUPIED_AREA_AND_RECT success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest22, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_OCCUPIED_AREA_AND_RECT);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: OnRemoteRequest23
 * @tc.desc: test TRANS_ID_UPDATE_ACTIVE_STATUS success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest23, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_ACTIVE_STATUS);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest24
 * @tc.desc: test TRANS_ID_GET_WINDOW_PROPERTY success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest24, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_GET_WINDOW_PROPERTY);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest25
 * @tc.desc: test TRANS_ID_NOTIFY_OUTSIDE_PRESSED success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest25, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_OUTSIDE_PRESSED);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest26
 * @tc.desc: test TRANS_ID_NOTIFY_SCREEN_SHOT success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest26, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_SCREEN_SHOT);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest_TRANS_ID_NOTIFY_SCREEN_SHOT
 * @tc.desc: test TRANS_ID_NOTIFY_SCREEN_SHOT
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest_TRANS_ID_NOTIFY_SCREEN_SHOT, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_SCREEN_SHOT_APP_EVENT);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteInt32(0);
    data.WriteInterfaceToken(WindowStub::GetDescriptor());
    code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_SCREEN_SHOT_APP_EVENT);
    res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_TRANSACTION_FAILED);
}

/**
 * @tc.name: OnRemoteRequest27
 * @tc.desc: test TRANS_ID_NOTIFY_DESTROY success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest27, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_DESTROY);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest28
 * @tc.desc: test TRANS_ID_NOTIFY_FOREGROUND success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest28, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_FOREGROUND);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest29
 * @tc.desc: test TRANS_ID_NOTIFY_BACKGROUND success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest29, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_BACKGROUND);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest30
 * @tc.desc: test TRANS_ID_DUMP_INFO success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest30, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_DUMP_INFO);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest31
 * @tc.desc: test TRANS_ID_NOTIFY_CLIENT_POINT_UP success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest31, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_CLIENT_POINT_UP);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: OnRemoteRequest32
 * @tc.desc: test TRANS_ID_UPDATE_ZOOM_TRANSFORM success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest32, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_ZOOM_TRANSFORM);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest33
 * @tc.desc: test TRANS_ID_RESTORE_SPLIT_WINDOW_MODE success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest33, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_RESTORE_SPLIT_WINDOW_MODE);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: OnRemoteRequest34
 * @tc.desc: test TRANS_ID_CONSUME_KEY_EVENT success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest34, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_CONSUME_KEY_EVENT);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, static_cast<int>(ERR_INVALID_DATA));
}

/**
 * @tc.name: OnRemoteRequest35
 * @tc.desc: test TRANS_ID_NOTIFY_FOREGROUND_INTERACTIVE_STATUS success
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest35, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_NOTIFY_FOREGROUND_INTERACTIVE_STATUS);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 0);
}

/**
 * @tc.name: OnRemoteRequest36
 * @tc.desc: test TRANS_ID_UPDATE_DRAG_EVENT fail
 * @tc.type: FUNC
 */
HWTEST_F(WindowStubTest, OnRemoteRequest36, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    data.WriteInterfaceToken(WindowStub::GetDescriptor());
    data.WriteInt32(0);
    data.WriteInt32(0);
    data.WriteUint32(100);

    uint32_t code = static_cast<uint32_t>(IWindow::WindowMessage::TRANS_ID_UPDATE_DRAG_EVENT);
    int res = windowStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, 5);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
