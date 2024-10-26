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

#include "session/container/include/zidl/session_stage_stub.h"
#include "session/container/include/zidl/session_stage_ipc_interface_code.h"
#include <ipc_types.h>
#include <iremote_stub.h>

#include "iremote_object_mocker.h"
#include "mock/mock_session_stage.h"
#include <message_option.h>
#include <message_parcel.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "session_manager/include/scene_session_manager.h"
#include "session_manager/include/zidl/scene_session_manager_interface.h"
#include "window_manager.h"
#include "window_manager_agent.h"
#include "ws_common.h"
#include "zidl/window_manager_agent_interface.h"
#include "window_manager_hilog.h"


using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionStageStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<SessionStageStub> sessionStageStub_ = new SessionStageMocker();
};

void SessionStageStubTest::SetUpTestCase()
{
}

void SessionStageStubTest::TearDownTestCase()
{
}

void SessionStageStubTest::SetUp()
{
}

void SessionStageStubTest::TearDown()
{
}

namespace {
/**
 * @tc.name: OnRemoteRequest
 * @tc.desc: test function : OnRemoteRequest
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, OnRemoteRequest, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = new WindowManagerAgent();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_UNREGISTER_WINDOW_MANAGER_AGENT);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(22, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleSetActive
 * @tc.desc: test function : HandleSetActive
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSetActive, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteBool(false);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleSetActive(data, reply));
}

/**
 * @tc.name: HandleUpdateRect
 * @tc.desc: test function : HandleUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateRect, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteUint32(1);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleUpdateRect(data, reply));
}

/**
 * @tc.name: HandleBackEventInner
 * @tc.desc: test function : HandleBackEventInner
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleBackEventInner, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteBool(false);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleBackEventInner(data, reply));
}

/**
 * @tc.name: HandleUpdateFocus
 * @tc.desc: test function : HandleUpdateFocus
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateFocus, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteBool(false);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleUpdateFocus(data, reply));
}

/**
 * @tc.name: HandleNotifyDestroy
 * @tc.desc: test function : HandleNotifyDestroy
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyDestroy, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;

    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleNotifyDestroy(data, reply));
}

/**
 * @tc.name: HandleNotifyTransferComponentData
 * @tc.desc: test function : HandleNotifyTransferComponentData
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyTransferComponentData, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteUint32(1);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(22, sessionStageStub_->HandleNotifyTransferComponentData(data, reply));
}

/**
 * @tc.name: HandleNotifyOccupiedAreaChange
 * @tc.desc: test function : HandleNotifyOccupiedAreaChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyOccupiedAreaChange, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteUint32(1);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(22, sessionStageStub_->HandleNotifyOccupiedAreaChange(data, reply));
}

/**
 * @tc.name: HandleUpdateAvoidArea
 * @tc.desc: test function : HandleUpdateAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateAvoidArea, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteUint32(1);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(22, sessionStageStub_->HandleUpdateAvoidArea(data, reply));
}

/**
 * @tc.name: HandleNotifyScreenshot
 * @tc.desc: test function : HandleNotifyScreenshot
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyScreenshot, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;

    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleNotifyScreenshot(data, reply));
}

/**
 * @tc.name: HandleDumpSessionElementInfo
 * @tc.desc: test function : HandleDumpSessionElementInfo
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleDumpSessionElementInfo, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;

    std::vector<std::string> params;
    params.push_back("test1");
    params.push_back("test2");
    data.WriteStringVector(params);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleDumpSessionElementInfo(data, reply));
}

/**
 * @tc.name: HandleNotifyTouchOutside
 * @tc.desc: test function : HandleNotifyTouchOutside
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyTouchOutside, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;

    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleNotifyTouchOutside(data, reply));
}

/**
 * @tc.name: HandleUpdateWindowMode
 * @tc.desc: test function : HandleUpdateWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateWindowMode, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteUint32(1);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleUpdateWindowMode(data, reply));
}

/**
 * @tc.name: NotifySessionFullScreen
 * @tc.desc: test function : NotifySessionFullScreen
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifySessionFullScreen, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteBool(true);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    ASSERT_EQ(0, sessionStageStub_->HandleNotifySessionFullScreen(data, reply));
}

/**
 * @tc.name: HandleNotifyWindowVisibilityChange
 * @tc.desc: test function : HandleNotifyWindowVisibilityChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyWindowVisibilityChange, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleNotifyWindowVisibilityChange(data, reply));
}

/**
 * @tc.name: HandleUpdateDensity
 * @tc.desc: test function : HandleUpdateDensity
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateDensity, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleNotifyWindowVisibilityChange(data, reply));
}

/**
 * @tc.name: HandleUpdateOrientation
 * @tc.desc: test function : HandleUpdateOrientation
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateOrientation, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleUpdateOrientation(data, reply));
}

/**
 * @tc.name: HandleNotifyCloseExistPipWindow
 * @tc.desc: test function : HandleNotifyCloseExistPipWindow
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyCloseExistPipWindow, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleNotifyCloseExistPipWindow(data, reply));
}

/**
 * @tc.name: HandleUpdateMaximizeMode
 * @tc.desc: test function : HandleUpdateMaximizeMode
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateMaximizeMode, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleUpdateMaximizeMode(data, reply));
}

/**
 * @tc.name: HandleUpdateTitleInTargetPos
 * @tc.desc: test function : HandleUpdateTitleInTargetPos
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateTitleInTargetPos, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleUpdateTitleInTargetPos(data, reply));
}

/**
 * @tc.name: HandleNotifyDensityFollowHost
 * @tc.desc: test function : HandleNotifyDensityFollowHost
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyDensityFollowHost, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleNotifyDensityFollowHost(data, reply));
}

/**
 * @tc.name: HandleGetUIContentRemoteObj
 * @tc.desc: test function : HandleGetUIContentRemoteObj
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleGetUIContentRemoteObj, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_NE(sessionStageStub_, nullptr);
    ASSERT_EQ(0, sessionStageStub_->HandleGetUIContentRemoteObj(data, reply));
}

/**
 * @tc.name: HandleSetPipActionEvent
 * @tc.desc: test function : HandleSetPipActionEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSetPipActionEvent, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteString("str");
    data.WriteInt32(1);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleSetPipActionEvent(data, reply));
}

/**
 * @tc.name: HandleSetPiPControlEvent
 * @tc.desc: test function : HandleSetPiPControlEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSetPiPControlEvent, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto controlType = WsPiPControlType::VIDEO_PLAY_PAUSE;
    auto status = WsPiPControlStatus::PLAY;
    data.WriteUint32(static_cast<uint32_t>(controlType));
    data.WriteInt32(static_cast<int32_t>(status));
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    ASSERT_EQ(0, sessionStageStub_->HandleSetPiPControlEvent(data, reply));
}

/**
 * @tc.name: HandleSetUniqueVirtualPixelRatio
 * @tc.desc: test function : HandleSetUniqueVirtualPixelRatio
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSetUniqueVirtualPixelRatio, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    data.WriteFloat(3.25f);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    ASSERT_EQ(0, sessionStageStub_->HandleSetUniqueVirtualPixelRatio(data, reply));
}

/**
 * @tc.name: HandleNotifyCompatibleModeEnableInPad
 * @tc.desc: test function : HandleNotifyCompatibleModeEnableInPad
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyCompatibleModeEnableInPad, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_COMPATIBLE_MODE_ENABLE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteBool(true);
    ASSERT_NE(sessionStageStub_, nullptr);
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}
}
}
}