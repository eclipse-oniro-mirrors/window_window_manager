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

#include "mock/mock_session_stage.h"
#include <message_option.h>
#include <message_parcel.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "session_manager/include/zidl/scene_session_manager_interface.h"
#include "window_manager.h"
#include "window_manager_agent.h"
#include "ws_common.h"
#include "zidl/window_manager_agent_interface.h"
#include "feature/window_keyframe/rs_window_keyframe_node.h"

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
    sptr<SessionStageStub> sessionStageStub_ = sptr<SessionStageMocker>::MakeSptr();
};

void SessionStageStubTest::SetUpTestCase() {}

void SessionStageStubTest::TearDownTestCase() {}

void SessionStageStubTest::SetUp() {}

void SessionStageStubTest::TearDown() {}

namespace {
/**
 * @tc.name: OnRemoteRequest
 * @tc.desc: test function : OnRemoteRequest
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, OnRemoteRequest, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    data.WriteUint32(static_cast<uint32_t>(type));
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    data.WriteRemoteObject(windowManagerAgent->AsObject());

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_UNREGISTER_WINDOW_MANAGER_AGENT);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(22, sessionStageStub_->OnRemoteRequest(code, data, reply, option));

    MessageParcel errorData;
    errorData.WriteInterfaceToken(u"OHOS.ISessionStage.TEST");
    code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_ACTIVE);
    ASSERT_EQ(ERR_TRANSACTION_FAILED, sessionStageStub_->OnRemoteRequest(code, errorData, reply, option));
}

/**
 * @tc.name: HandleSetActive
 * @tc.desc: test function : HandleSetActive
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSetActive, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_ACTIVE);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateRect
 * @tc.desc: test function : HandleUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateRect, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteBool(false);
    sptr<SceneAnimationConfig> animationConfigInfo = sptr<SceneAnimationConfig>::MakeSptr();
    data.WriteParcelable(animationConfigInfo);
    data.WriteBool(false);
    data.WriteUint32(0);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SIZE_CHANGE);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateRect
 * @tc.desc: test function : HandleUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateRect01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteBool(false);
    sptr<SceneAnimationConfig> animationConfigInfo = nullptr;
    data.WriteParcelable(animationConfigInfo);
    data.WriteBool(false);
    data.WriteUint32(0);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SIZE_CHANGE);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(-1, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateRect
 * @tc.desc: test function : HandleUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateRect02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteBool(false);
    sptr<SceneAnimationConfig> animationConfigInfo = sptr<SceneAnimationConfig>::MakeSptr();
    data.WriteParcelable(animationConfigInfo);
    data.WriteBool(true);
    data.WriteUint32(0);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SIZE_CHANGE);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateRect
 * @tc.desc: test function : HandleUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateRect03, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteBool(false);
    sptr<SceneAnimationConfig> animationConfigInfo = sptr<SceneAnimationConfig>::MakeSptr();
    data.WriteParcelable(animationConfigInfo);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SIZE_CHANGE);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(-1, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateRect
 * @tc.desc: test function : HandleUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateRect04, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteBool(true);
    sptr<SceneAnimationConfig> animationConfigInfo = sptr<SceneAnimationConfig>::MakeSptr();
    data.WriteParcelable(animationConfigInfo);
    data.WriteBool(false);
    data.WriteUint32(0);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SIZE_CHANGE);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(-1, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateRect
 * @tc.desc: test function : HandleUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateRect05, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteBool(true);
    sptr<SceneAnimationConfig> animationConfigInfo = sptr<SceneAnimationConfig>::MakeSptr();
    data.WriteParcelable(animationConfigInfo);
    data.WriteBool(true);
    data.WriteUint32(0);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SIZE_CHANGE);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateSessionViewportConfig
 * @tc.desc: test function : HandleUpdateSessionViewportConfig
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateSessionViewportConfig, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(false);
    data.WriteFloat(1.0f);
    data.WriteUint64(0);
    data.WriteInt32(0);
    data.WriteUint32(0);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleUpdateSessionViewportConfig(data, reply));
}

/**
 * @tc.name: HandleNotifySecureLimitChange
 * @tc.desc: test function : HandleNotifySecureLimitChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifySecureLimitChange, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->HandleNotifySecureLimitChange(data, reply));
}

/**
 * @tc.name: HandleUpdateDensity01
 * @tc.desc: test function : HandleUpdateDensity
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateDensity01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DENSITY_CHANGE);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateDensity02
 * @tc.desc: test function : HandleUpdateDensity
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateDensity02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    ASSERT_EQ(0, sessionStageStub_->HandleUpdateDensity(data, reply));
}

/**
 * @tc.name: HandleUpdateOrientation
 * @tc.desc: test function : HandleUpdateOrientation
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateOrientation, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_ORIENTATION_CHANGE);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleBackEventInner
 * @tc.desc: test function : HandleBackEventInner
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleBackEventInner, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_HANDLE_BACK_EVENT);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyDestroy
 * @tc.desc: test function : HandleNotifyDestroy
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyDestroy, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DESTROY);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateFocus
 * @tc.desc: test function : HandleUpdateFocus
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateFocus, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_FOCUS_CHANGE);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(5, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateFocus01
 * @tc.desc: test function : HandleUpdateFocus
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateFocus01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    auto info = sptr<FocusNotifyInfo>::MakeSptr();
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteParcelable(info);
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_FOCUS_CHANGE);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyTransferComponentData
 * @tc.desc: test function : HandleNotifyTransferComponentData
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyTransferComponentData, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteUint32(1);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TRANSFER_COMPONENT_DATA);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(ERR_INVALID_VALUE, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyTransferComponentDataSync
 * @tc.desc: test function : HandleNotifyTransferComponentDataSync
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyTransferComponentDataSync, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteUint32(1);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TRANSFER_COMPONENT_DATA_SYNC);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(static_cast<int>(WSErrorCode::WS_ERROR_TRANSFER_DATA_FAILED),
              sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyOccupiedAreaChange
 * @tc.desc: test function : HandleNotifyOccupiedAreaChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyOccupiedAreaChange, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteUint32(1);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_OCCUPIED_AREA_CHANGE_INFO);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(ERR_INVALID_VALUE, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyKeyboardAnimationCompleted
 * @tc.desc: test function : HandleNotifyKeyboardAnimationCompleted
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyKeyboardAnimationCompleted, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    sptr<KeyboardPanelInfo> keyboardPanelInfo = sptr<KeyboardPanelInfo>::MakeSptr();
    data.WriteParcelable(keyboardPanelInfo);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_KEYBOARD_ANIMATION_COMPLETED);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(ERR_NONE, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyKeyboardAnimationWillBeginInvalidParams
 * @tc.desc: test function : HandleNotifyKeyboardAnimationWillBeginInvalidParams
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyKeyboardAnimationWillBeginInvalidParams, Function | SmallTest | Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    int32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_KEYBOARD_ANIMATION_WILLBEGIN);
    ASSERT_EQ(ERR_INVALID_VALUE, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyKeyboardAnimationWillBegin
 * @tc.desc: test function : HandleNotifyKeyboardAnimationWillBegin
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyKeyboardAnimationWillBegin, Function | SmallTest | Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    int32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_KEYBOARD_ANIMATION_WILLBEGIN);
    sptr<KeyboardAnimationInfo> keyboardAnimationInfo = sptr<KeyboardAnimationInfo>::MakeSptr();
    data.WriteParcelable(keyboardAnimationInfo);
    ASSERT_EQ(ERR_NONE, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateAvoidArea
 * @tc.desc: test function : HandleUpdateAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateAvoidArea, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteUint32(1);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_UPDATE_AVOID_AREA);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(ERR_INVALID_VALUE, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyScreenshot
 * @tc.desc: test function : HandleNotifyScreenshot
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyScreenshot, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SCREEN_SHOT);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyScreenshotAppEvent
 * @tc.desc: test function : HandleNotifyScreenshotAppEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyScreenshotAppEvent, TestSize.Level1)
{
    ASSERT_NE(nullptr, sessionStageStub_);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SCREEN_SHOT_APP_EVENT);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    EXPECT_EQ(22, sessionStageStub_->OnRemoteRequest(code, data, reply, option));

    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteInt32(1);
    EXPECT_EQ(ERR_NONE, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
    int32_t ret = reply.ReadInt32();
    EXPECT_EQ(ret, ERR_NONE);
}

/**
 * @tc.name: HandleDumpSessionElementInfo
 * @tc.desc: test function : HandleDumpSessionElementInfo
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleDumpSessionElementInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    std::vector<std::string> params;
    params.push_back("test1");
    params.push_back("test2");
    data.WriteStringVector(params);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_DUMP_SESSSION_ELEMENT_INFO);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyTouchOutside
 * @tc.desc: test function : HandleNotifyTouchOutside
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyTouchOutside, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TOUCH_OUTSIDE);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateWindowMode
 * @tc.desc: test function : HandleUpdateWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateWindowMode, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteUint32(1);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_WINDOW_MODE_CHANGE);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyLayoutFinishAfterWindowModeChange
 * @tc.desc: test function : HandleNotifyLayoutFinishAfterWindowModeChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyLayoutFinishAfterWindowModeChange, TestSize.Level1)
{
    ASSERT_NE(nullptr, sessionStageStub_);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code =
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_LAYOUT_FINISH_AFTER_WINDOW_MODE_CHANGE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->OnRemoteRequest(code, data, reply, option));

    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteUint32(1);
    EXPECT_EQ(ERR_NONE, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
    int32_t ret = reply.ReadInt32();
    EXPECT_EQ(ret, static_cast<int32_t>(WSError::WS_OK));
}

/**
 * @tc.name: HandleNotifyForegroundInteractiveStatus
 * @tc.desc: test function : HandleNotifyForegroundInteractiveStatus
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyForegroundInteractiveStatus, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteBool(false);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_FOREGROUND_INTERACTIVE_STATUS);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateMaximizeMode
 * @tc.desc: test function : HandleUpdateMaximizeMode
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateMaximizeMode, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_MAXIMIZE_MODE_CHANGE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyCloseExistPipWindow
 * @tc.desc: test function : HandleNotifyCloseExistPipWindow
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyCloseExistPipWindow, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_CLOSE_EXIST_PIP_WINDOW);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifySessionForeground
 * @tc.desc: test function : HandleNotifySessionForeground
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifySessionForeground, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SESSION_FOREGROUND);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteUint32(1);
    data.WriteBool(false);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifySessionBackground
 * @tc.desc: test function : HandleNotifySessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifySessionBackground, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SESSION_BACKGROUND);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteUint32(1);
    data.WriteBool(false);
    data.WriteBool(false);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateTitleInTargetPos
 * @tc.desc: test function : HandleUpdateTitleInTargetPos
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateTitleInTargetPos, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TITLE_POSITION_CHANGE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyDensityFollowHost
 * @tc.desc: test function : HandleNotifyDensityFollowHost
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyDensityFollowHost, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DENSITY_FOLLOW_HOST);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyWindowVisibilityChange
 * @tc.desc: test function : HandleNotifyWindowVisibilityChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyWindowVisibilityChange, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_WINDOW_VISIBILITY_CHANGE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteBool(true);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyWindowOcclusionState
 * @tc.desc: test function : HandleNotifyWindowOcclusionState
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyWindowOcclusionState, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_WINDOW_OCCLUSION_STATE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(WindowVisibilityState::END) + 1);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    EXPECT_EQ(sessionStageStub_->OnRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleNotifyTransformChange
 * @tc.desc: test function : HandleNotifyTransformChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyTransformChange, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_TRANSFORM_CHANGE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteBool(true);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyDialogStateChange
 * @tc.desc: test function : HandleNotifyDialogStateChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyDialogStateChange, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DIALOG_STATE_CHANGE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteBool(true);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyGlobalScaledRectChange
 * @tc.desc: test function : HandleNotifyGlobalScaledRectChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyGlobalScaledRectChange, TestSize.Level1)
{
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_GLOBAL_SCALED_RECT);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    Rect globalScaledRect = { 0, 0, 1000, 1000 };
    globalScaledRect.Marshalling(data);
    EXPECT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleSetPipActionEvent
 * @tc.desc: test function : HandleSetPipActionEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSetPipActionEvent, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_PIP_ACTION_EVENT);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteString("str");
    data.WriteInt32(1);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleSetPiPControlEvent
 * @tc.desc: test function : HandleSetPiPControlEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSetPiPControlEvent, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_PIP_CONTROL_EVENT);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    auto controlType = WsPiPControlType::VIDEO_PLAY_PAUSE;
    auto status = WsPiPControlStatus::PLAY;
    data.WriteUint32(static_cast<uint32_t>(controlType));
    data.WriteInt32(static_cast<int32_t>(status));
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateDisplayId
 * @tc.desc: test function : HandleUpdateDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateDisplayId, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DISPLAYID_CHANGE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteUint64(2);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyDisplayMove
 * @tc.desc: test function : HandleNotifyDisplayMove
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyDisplayMove, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DISPLAY_MOVE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteUint64(0); // from
    data.WriteUint64(1); // to
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleSwitchFreeMultiWindow
 * @tc.desc: test function : HandleSwitchFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSwitchFreeMultiWindow, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SWITCH_FREEMULTIWINDOW);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteBool(true); // enable
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleGetUIContentRemoteObj
 * @tc.desc: test function : HandleGetUIContentRemoteObj
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleGetUIContentRemoteObj, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_GET_UI_CONTENT_REMOTE_OBJ);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    ASSERT_NE(sessionStageStub_, nullptr);
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyKeyboardPanelInfoChange
 * @tc.desc: test function : HandleNotifyKeyboardPanelInfoChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyKeyboardPanelInfoChange, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_KEYBOARD_INFO_CHANGE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    sptr<KeyboardPanelInfo> keyboardPanelInfo = sptr<KeyboardPanelInfo>::MakeSptr();
    data.WriteParcelable(keyboardPanelInfo);
    ASSERT_NE(sessionStageStub_, nullptr);
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandlePcAppInPadNormalClose
 * @tc.desc: test function : HandlePcAppInPadNormalClose
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandlePcAppInPadNormalClose, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_PCAPPINPADNORMAL_CLOSE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    ASSERT_NE(sessionStageStub_, nullptr);
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleSetUniqueVirtualPixelRatio
 * @tc.desc: test function : HandleSetUniqueVirtualPixelRatio
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSetUniqueVirtualPixelRatio, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DENSITY_UNIQUE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteBool(true);
    data.WriteFloat(3.25f);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateAnimationSpeed
 * @tc.desc: test function : HandleUpdateAnimationSpeed
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateAnimationSpeed, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    ASSERT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleUpdateAnimationSpeed(data, reply));
    data.WriteFloat(2.0f);
    ASSERT_EQ(0, sessionStageStub_->HandleUpdateAnimationSpeed(data, reply));
}

/**
 * @tc.name: HandleNotifySessionFullScreen
 * @tc.desc: test function : HandleNotifySessionFullScreen
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifySessionFullScreen, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_SESSION_FULLSCREEN);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteBool(true);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
    data.WriteBool(false);
    ASSERT_EQ(0, sessionStageStub_->HandleNotifySessionFullScreen(data, reply));
}

/**
 * @tc.name: HandleNotifyCompatibleModePropertyChange
 * @tc.desc: test function : HandleNotifyCompatibleModePropertyChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyCompatibleModePropertyChange, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_COMPATIBLE_MODE_PROPERTY_CHANGE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    data.WriteParcelable(compatibleModeProperty.GetRefPtr());
    ASSERT_NE(sessionStageStub_, nullptr);
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyDumpInfo
 * @tc.desc: test function : HandleNotifyDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyDumpInfo, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_DUMP_INFO);
    std::vector<std::string> params;
    std::vector<std::string> info;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteStringVector(params);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleSetSplitButtonVisible
 * @tc.desc: test function : HandleSetSplitButtonVisible
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSetSplitButtonVisible, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_SPLIT_BUTTON_VISIBLE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteBool(true);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
    data.WriteBool(false);
    ASSERT_EQ(0, sessionStageStub_->HandleSetSplitButtonVisible(data, reply));
}

/**
 * @tc.name: HandleSetEnableDragBySystem
 * @tc.desc: test function : HandleSetEnableDragBySystem
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSetEnableDragBySystem, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_ENABLE_DRAG_BY_SYSTEM);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteBool(true);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
    data.WriteBool(false);
    ASSERT_EQ(0, sessionStageStub_->HandleSetEnableDragBySystem(data, reply));
}

/**
 * @tc.name: HandleSetFullScreenWaterfallMode
 * @tc.desc: test function : HandleSetFullScreenWaterfallMode
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSetFullScreenWaterfallMode, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_FULLSCREEN_WATERFALL_MODE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteBool(true);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
    data.WriteBool(false);
    ASSERT_EQ(0, sessionStageStub_->HandleSetFullScreenWaterfallMode(data, reply));
}

/**
 * @tc.name: HandleSetSupportEnterWaterfallMode
 * @tc.desc: test function : HandleSetSupportEnterWaterfallMode
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSetSupportEnterWaterfallMode, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_SUPPORT_ENTER_WATERFALL_MODE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteBool(true);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
    data.WriteBool(false);
    ASSERT_EQ(0, sessionStageStub_->HandleSetSupportEnterWaterfallMode(data, reply));
}

/**
 * @tc.name: HandleSendContainerModalEvent
 * @tc.desc: test function : HandleSendContainerModalEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSendContainerModalEvent, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SEND_CONTAINER_MODAL_EVENT);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteString("name");
    data.WriteString("value");
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
    data.WriteString("name1");
    data.WriteString("value2");
    ASSERT_EQ(0, sessionStageStub_->HandleSendContainerModalEvent(data, reply));
}

/**
 * @tc.name: HandleNotifyHighlightChange
 * @tc.desc: test function : HandleNotifyHighlightChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyHighlightChange, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteBool(false);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    ASSERT_EQ(5, sessionStageStub_->HandleNotifyHighlightChange(data, reply));
}

/**
 * @tc.name: HandleNotifyHighlightChange01
 * @tc.desc: test function : HandleNotifyHighlightChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyHighlightChange01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    auto info = sptr<HighlightNotifyInfo>::MakeSptr();
    data.WriteParcelable(info);
    data.WriteBool(false);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    ASSERT_EQ(0, sessionStageStub_->HandleNotifyHighlightChange(data, reply));
}

/**
 * @tc.name: HandleNotifyWindowCrossAxisChange
 * @tc.desc: test function : HandleNotifyWindowCrossAxisChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyWindowCrossAxisChange, TestSize.Level1)
{
    sptr<SessionStageMocker> sessionStageStub = sptr<SessionStageMocker>::MakeSptr();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    CrossAxisState state = CrossAxisState::STATE_CROSS;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_CROSS_AXIS);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(state));
    EXPECT_CALL(*sessionStageStub, NotifyWindowCrossAxisChange(state)).Times(1);
    sessionStageStub->OnRemoteRequest(code, data, reply, option);
    MessageParcel data2;
    data2.WriteBool(true);
    EXPECT_EQ(sessionStageStub->HandleNotifyWindowCrossAxisChange(data, reply), ERR_INVALID_DATA);
    MessageParcel data3;
    data2.WriteUint32(1000);
    EXPECT_EQ(sessionStageStub->HandleNotifyWindowCrossAxisChange(data, reply), ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleNotifyPiPActiveStatusChange
 * @tc.desc: test function : HandleNotifyPiPActiveStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyPiPActiveStatusChange, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_ACTIVE_STATUS_CHANGE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteBool(true);
    ASSERT_EQ(ERR_NONE, sessionStageStub_->OnRemoteRequest(code, data, reply, option));

    MessageParcel data1;
    data1.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    EXPECT_EQ(sessionStageStub_->OnRemoteRequest(code, data1, reply, option), ERR_INVALID_VALUE);
}

/**
 * @tc.name: HandleNotifyRotationChange
 * @tc.desc: test function : HandleNotifyRotationChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyRotationChange, Function | SmallTest | Level1)
{
    sptr<SessionStageMocker> sessionStageStub = sptr<SessionStageMocker>::MakeSptr();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    RotationChangeInfo info = { RotationChangeType::WINDOW_WILL_ROTATE, 0, 0, { 0, 0, 2720, 1270 } };
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_ROTATION_CHANGE);
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(info.type_));
    data.WriteUint32(info.orientation_);
    data.WriteUint64(info.displayId_);
    data.WriteInt32(info.displayRect_.posX_);
    data.WriteInt32(info.displayRect_.posY_);
    data.WriteUint32(info.displayRect_.width_);
    data.WriteUint32(info.displayRect_.height_);
    sessionStageStub->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(sessionStageStub_->HandleNotifyRotationChange(data, reply), ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleLinkKeyFrameNode
 * @tc.desc: test function : HandleLinkKeyFrameNode
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleLinkKeyFrameNode, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_LINK_KEYFRAME_NODE);
    EXPECT_TRUE(sessionStageStub_ != nullptr);
    EXPECT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleSetStageKeyFramePolicy
 * @tc.desc: test function : HandleSetStageKeyFramePolicy
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSetStageKeyFramePolicy, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    KeyFramePolicy keyFramePolicy;
    data.WriteParcelable(&keyFramePolicy);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_STAGE_KEYFRAME_POLICY);
    EXPECT_TRUE(sessionStageStub_ != nullptr);
    EXPECT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyRotationProperty
 * @tc.desc: test function : HandleNotifyRotationProperty
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyRotationProperty, Function | SmallTest | Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteInt32(0);
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteUint32(0);
    
    data.WriteInt32(0);
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteUint32(0);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_ROTATION_PROPERTY);
    EXPECT_TRUE(sessionStageStub_ != nullptr);
    EXPECT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyPageRotationIsIgnored
 * @tc.desc: test function : HandleNotifyPageRotationIsIgnored
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyPageRotationIsIgnored, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_PAGE_ROTATION_ISIGNORED);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    EXPECT_EQ(ERR_NONE, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyAppForceLandscapeConfigUpdated
 * @tc.desc: test function : HandleNotifyAppForceLandscapeConfigUpdated
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyAppForceLandscapeConfigUpdated, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    EXPECT_EQ(0, sessionStageStub_->HandleNotifyAppForceLandscapeConfigUpdated(data, reply));
}

/**
 * @tc.name: HandleNotifyAppForceLandscapeConfigUpdated01
 * @tc.desc: test function : HandleNotifyAppForceLandscapeConfigUpdated
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyAppForceLandscapeConfigUpdated01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    uint32_t code =
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_APP_FORCE_LANDSCAPE_CONFIG_UPDATED);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(0, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleCloseSpecificScene
 * @tc.desc: test function : HandleCloseSpecificScene
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleCloseSpecificScene, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    EXPECT_EQ(ERR_NONE, sessionStageStub_->HandleCloseSpecificScene(data, reply));
}

/**
 * @tc.name: HandleUpdateWindowModeForUITest01
 * @tc.desc: test function : HandleUpdateWindowModeForUITest
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateWindowModeForUITest01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->HandleUpdateWindowModeForUITest(data, reply));

    data.WriteInt32(0);
    EXPECT_EQ(ERR_NONE, sessionStageStub_->HandleUpdateWindowModeForUITest(data, reply));
}

/**
 * @tc.name: HandleSendFbActionEvent
 * @tc.desc: test function : HandleSendFbActionEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSendFbActionEvent, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    sptr<KeyboardPanelInfo> keyboardPanelInfo = sptr<KeyboardPanelInfo>::MakeSptr();

    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SEND_FB_ACTION_EVENT);
    ASSERT_TRUE((sessionStageStub_ != nullptr));
    ASSERT_EQ(ERR_INVALID_VALUE, sessionStageStub_->OnRemoteRequest(code, data, reply, option));

    MessageParcel data1;
    std::string action = "SendFbActionEvent";
    data1.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data1.WriteString(action);
    code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SEND_FB_ACTION_EVENT);
    ASSERT_EQ(ERR_NONE, sessionStageStub_->OnRemoteRequest(code, data1, reply, option));
}

/**
 * @tc.name: HandleUpdateGlobalDisplayRectFromServerWithIncompleteRect
 * @tc.desc: Verify that HandleUpdateGlobalDisplayRectFromServer returns error when rect data is incomplete
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateGlobalDisplayRectFromServerWithIncompleteRect, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    // Missing width and height fields
    data.WriteInt32(10); // posX
    data.WriteInt32(20); // posY

    int result = sessionStageStub_->HandleUpdateGlobalDisplayRectFromServer(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleUpdateGlobalDisplayRectFromServerWithInvalidReason
 * @tc.desc: Verify that HandleUpdateGlobalDisplayRectFromServer rejects missing or invalid reason values
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateGlobalDisplayRectFromServerWithInvalidReason, TestSize.Level1)
{
    auto writeRect = [](MessageParcel& data) {
        data.WriteInt32(10); // posX
        data.WriteInt32(20); // posY
        data.WriteInt32(100); // width
        data.WriteInt32(200); // height
    };

    // Case 1: Missing reason field
    {
        MessageParcel data;
        MessageParcel reply;
        writeRect(data);
        int result = sessionStageStub_->HandleUpdateGlobalDisplayRectFromServer(data, reply);
        EXPECT_EQ(result, ERR_INVALID_DATA);
    }

    // Case 2: reason < SizeChangeReason::UNDEFINED (invalid)
    {
        MessageParcel data;
        MessageParcel reply;
        writeRect(data);
        data.WriteUint32(static_cast<uint32_t>(SizeChangeReason::UNDEFINED) - 1);
        int result = sessionStageStub_->HandleUpdateGlobalDisplayRectFromServer(data, reply);
        EXPECT_EQ(result, ERR_INVALID_DATA);
    }

    // Case 3: reason == SizeChangeReason::END (boundary out-of-range)
    {
        MessageParcel data;
        MessageParcel reply;
        writeRect(data);
        data.WriteUint32(static_cast<uint32_t>(SizeChangeReason::END));
        int result = sessionStageStub_->HandleUpdateGlobalDisplayRectFromServer(data, reply);
        EXPECT_EQ(result, ERR_INVALID_DATA);
    }

    // Case 4: reason > SizeChangeReason::END (overflow)
    {
        MessageParcel data;
        MessageParcel reply;
        writeRect(data);
        data.WriteUint32(static_cast<uint32_t>(SizeChangeReason::END) + 1);
        int result = sessionStageStub_->HandleUpdateGlobalDisplayRectFromServer(data, reply);
        EXPECT_EQ(result, ERR_INVALID_DATA);
    }
}

/**
 * @tc.name: HandleUpdateGlobalDisplayRectFromServerSuccess
 * @tc.desc: Verify HandleUpdateGlobalDisplayRectFromServer accepts valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateGlobalDisplayRectFromServerSuccess, TestSize.Level1)
{
    constexpr int32_t posX = 10;
    constexpr int32_t posY = 20;
    constexpr int32_t width = 100;
    constexpr int32_t height = 200;
    constexpr SizeChangeReason reason = SizeChangeReason::UNDEFINED;

    MessageParcel data;
    MessageParcel reply;

    // Write valid rect and reason
    data.WriteInt32(posX);
    data.WriteInt32(posY);
    data.WriteInt32(width);
    data.WriteInt32(height);
    data.WriteUint32(static_cast<uint32_t>(reason));

    int result = sessionStageStub_->HandleUpdateGlobalDisplayRectFromServer(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleNotifyAppHookWindowInfoUpdated
 * @tc.desc: test function : HandleNotifyAppHookWindowInfoUpdated
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleNotifyAppHookWindowInfoUpdated, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_APP_HOOK_WINDOW_INFO_UPDATED);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    EXPECT_EQ(ERR_NONE, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateIsShowDecorInFreeMultiWindow
 * @tc.desc: test function : HandleUpdateIsShowDecorInFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateIsShowDecorInFreeMultiWindow, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();
    data.WriteRemoteObject(windowManagerAgent->AsObject());
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    uint32_t code =
        static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_NOTIFY_UPDATE_SHOW_DECOR_IN_FREE_MULTI_WINDOW);
    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateIsShowDecorInFreeMultiWindow 02
 * @tc.desc: test function : HandleUpdateIsShowDecorInFreeMultiWindow 02
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateIsShowDecorInFreeMultiWindow02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    bool isShow = true;
    data.WriteBool(static_cast<bool>(isShow));
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    EXPECT_EQ(ERR_NONE, sessionStageStub_->HandleUpdateIsShowDecorInFreeMultiWindow(data, reply));
}

/**
 * @tc.name: HandleUpdateBrightness
 * @tc.desc: test function : HandleUpdateBrightness
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleUpdateBrightness, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    bool isShow = true;
    data.WriteFloat(1.0f);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    EXPECT_EQ(ERR_NONE, sessionStageStub_->HandleUpdateBrightness(data, reply));
}

/**
 * @tc.name: HandleAddSidebarBlur
 * @tc.desc: test function : HandleAddSidebarBlur
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleAddSidebarBlur, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_ADD_SIDEBAR_BLUR);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    EXPECT_EQ(ERR_NONE, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleSetSidebarBlurStyleWithType01
 * @tc.desc: test function : HandleSetSidebarBlurStyleWithType
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSetSidebarBlurStyleWithType01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteUint32(0);
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_SIDEBAR_BLUR_STYLE);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    EXPECT_EQ(ERR_NONE, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleSetSidebarBlurStyleWithType02
 * @tc.desc: test function : HandleSetSidebarBlurStyleWithType
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSetSidebarBlurStyleWithType02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_SIDEBAR_BLUR_STYLE);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleSetSidebarBlurStyleWithType03
 * @tc.desc: test function : HandleSetSidebarBlurStyleWithType
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageStubTest, HandleSetSidebarBlurStyleWithType03, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SessionStageStub::GetDescriptor());
    data.WriteUint32(static_cast<uint32_t>(SidebarBlurType::END));
    uint32_t code = static_cast<uint32_t>(SessionStageInterfaceCode::TRANS_ID_SET_SIDEBAR_BLUR_STYLE);
    ASSERT_TRUE(sessionStageStub_ != nullptr);
    EXPECT_EQ(ERR_INVALID_DATA, sessionStageStub_->OnRemoteRequest(code, data, reply, option));
}
} // namespace
} // namespace Rosen
} // namespace OHOS