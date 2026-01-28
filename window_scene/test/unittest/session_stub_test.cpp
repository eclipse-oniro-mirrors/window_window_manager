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
#include <gmock/gmock.h>
#include <ipc_types.h>
#include <pointer_event.h>
#include <ui/rs_surface_node.h>
#include "ability_start_setting.h"
#include "iremote_object_mocker.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_session_stub.h"
#include "mock/mock_window_event_channel.h"
#include "mock_message_parcel.h"
#include "parcel/accessibility_event_info_parcel.h"
#include "session/host/include/zidl/session_ipc_interface_code.h"
#include "session/host/include/zidl/session_stub.h"
#include "transaction/rs_transaction.h"
#include "want.h"
#include "wm_common.h"
#include "ws_common.h"
#include "feature/window_keyframe/rs_window_keyframe_node.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const std::string UNDEFINED = "undefined";
const uint32_t MAX_INT = -1;
}

namespace OHOS::Accessibility {
class AccessibilityEventInfo;
}
class SessionStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    sptr<SessionStub> session_ = nullptr;
};

void SessionStubTest::SetUpTestCase() {}

void SessionStubTest::TearDownTestCase() {}

void SessionStubTest::SetUp()
{
    session_ = sptr<SessionStubMocker>::MakeSptr();
    EXPECT_NE(nullptr, session_);
}

void SessionStubTest::TearDown()
{
    session_ = nullptr;
}

namespace {
/**
 * @tc.name: OnRemoteRequest01
 * @tc.desc: sessionStub OnRemoteRequest01
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, OnRemoteRequest01, TestSize.Level1)
{
    uint32_t code = 1;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_SYNC };
    auto res = session_->OnRemoteRequest(code, data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    data.WriteInterfaceToken(u"OHOS.ISession");
    res = session_->OnRemoteRequest(code, data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: ProcessRemoteRequestTest01
 * @tc.desc: sessionStub ProcessRemoteRequestTest01
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, ProcessRemoteRequestTest01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_SYNC };
    data.WriteBool(true);
    auto res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_WINDOW_ANIMATION_FLAG), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);

    data.WriteBool(false);
    data.WriteBool(true);
    data.WriteString("");
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_FOREGROUND), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);

    data.WriteBool(true);
    data.WriteString("");
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_BACKGROUND), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);

    data.WriteBool(true);
    data.WriteString("");
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_DISCONNECT), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    sptr<IRemoteObjectMocker> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_NE(data.WriteRemoteObject(iRemoteObjectMocker), false);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_CONNECT), data, reply, option);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    AAFwk::Want options;
    EXPECT_NE(data.WriteString("HandleSessionException"), false);
    EXPECT_NE(data.WriteParcelable(&options), false);
    EXPECT_NE(data.WriteBool(false), false);
    EXPECT_NE(data.WriteInt32(33), false);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TERMINATE), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_SESSION_RECT), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RAISE_TO_APP_TOP), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: ProcessRemoteRequestTest02
 * @tc.desc: sessionStub ProcessRemoteRequestTest02
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, ProcessRemoteRequestTest02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_SYNC };
    data.WriteBool(true);
    auto res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_WINDOW_ANIMATION_FLAG), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RAISE_APP_MAIN_WINDOW), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    data.WriteBool(true);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_BACKPRESSED), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_MARK_PROCESSED), data, reply, option);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_MAXIMIZE_MODE), data, reply, option);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_MAXIMIZE_MODE), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NEED_AVOID), data, reply, option);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_AVOID_AREA), data, reply, option);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_ASPECT_RATIO), data, reply, option);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_CUSTOM_ANIMATION), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TRANSFER_EXTENSION_DATA), data, reply, option);
    ASSERT_EQ(ERR_INVALID_VALUE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_EXTENSION_DIED), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    data.WriteInt32(1);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_EXTENSION_TIMEOUT), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: ProcessRemoteRequestTest03
 * @tc.desc: sessionStub ProcessRemoteRequestTest03
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, ProcessRemoteRequestTest03, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_SYNC };
    data.WriteBool(true);
    sptr<IRemoteObjectMocker> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_NE(data.WriteRemoteObject(iRemoteObjectMocker), false);
    ASSERT_EQ(data.WriteUint32(1), true);
    AAFwk::Want options;
    EXPECT_NE(data.WriteString("HandleSessionException"), false);
    EXPECT_NE(data.WriteParcelable(&options), false);
    ASSERT_EQ(data.WriteUint64(2), true);
    auto res =
        session_->ProcessRemoteRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SHOW), data, reply, option);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    res =
        session_->ProcessRemoteRequest(static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_HIDE), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_DRAWING_COMPLETED), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_APP_REMOVE_STARTING_WINDOW), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_RECTCHANGE_LISTENER_REGISTERED),
        data,
        reply,
        option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_LANDSCAPE_MULTI_WINDOW), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_ACTIVE_PENDING_SESSION), data, reply, option);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    MessageParcel tmp;
    tmp.WriteUint32(1);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TRANSFER_ABILITY_RESULT), tmp, reply, option);
    ASSERT_EQ(ERR_INVALID_VALUE, res);
}

/**
 * @tc.name: ProcessRemoteRequestTest04
 * @tc.desc: sessionStub ProcessRemoteRequestTest04
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, ProcessRemoteRequestTest04, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_SYNC };
    ASSERT_EQ(data.WriteInt32(1), true);
    ASSERT_EQ(data.WriteInt32(2), true);
    auto res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_PROCESS_POINT_DOWN_SESSION), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SEND_POINTEREVENT_FOR_MOVE_DRAG), data, reply, option);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_CALLING_SESSION_ID), data, reply, option);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_CUSTOM_DECOR_HEIGHT), data, reply, option);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_ADJUST_KEYBOARD_LAYOUT), data, reply, option);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_ASYNC_ON), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_SYNC_ON), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TRIGGER_BIND_MODAL_UI_EXTENSION), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: ProcessRemoteRequestTest05
 * @tc.desc: sessionStub ProcessRemoteRequestTest05
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, ProcessRemoteRequestTest05, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_SYNC };
    data.WriteBool(true);
    sptr<IRemoteObjectMocker> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_NE(data.WriteRemoteObject(iRemoteObjectMocker), false);
    ASSERT_EQ(data.WriteUint32(1), true);
    AAFwk::Want options;
    EXPECT_NE(data.WriteString("HandleSessionException"), false);
    EXPECT_NE(data.WriteParcelable(&options), false);
    ASSERT_EQ(data.WriteUint64(2), true);
    auto res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_PIP_WINDOW_PREPARE_CLOSE), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_LAYOUT_FULL_SCREEN_CHANGE), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_RESTORE_MAIN_WINDOW), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_DIALOG_SESSION_BACKGESTURE_ENABLE),
        data,
        reply,
        option);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: ProcessRemoteRequestTest06
 * @tc.desc: sessionStub ProcessRemoteRequestTest06
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, ProcessRemoteRequestTest06, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_SYNC };
    AAFwk::Want want;
    data.WriteParcelable(&want);
    data.WriteBool(true);
    sptr<IRemoteObjectMocker> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_NE(data.WriteRemoteObject(iRemoteObjectMocker), false);
    ASSERT_EQ(data.WriteUint32(1), true);
    ASSERT_EQ(data.WriteUint32(1), true);
    AAFwk::Want options;
    EXPECT_NE(data.WriteString("HandleSessionException"), false);
    EXPECT_NE(data.WriteString("HandleSessionException"), false);
    EXPECT_NE(data.WriteParcelable(&options), false);
    ASSERT_EQ(data.WriteUint64(2), true);
    auto res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_EXCEPTION), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    ASSERT_EQ(data.WriteUint32(1), true);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_CALLING_SESSION_ID), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    ASSERT_EQ(data.WriteInt32(1), true);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_CUSTOM_DECOR_HEIGHT), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: ProcessRemoteRequestTest07
 * @tc.desc: sessionStub ProcessRemoteRequestTest07
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, ProcessRemoteRequestTest07, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_SYNC };
    ASSERT_EQ(data.WriteInt32(1), true);
    ASSERT_EQ(data.WriteInt32(1), true);
    ASSERT_EQ(data.WriteUint32(1), true);
    ASSERT_EQ(data.WriteUint32(1), true);
    ASSERT_EQ(data.WriteUint32(1), true);
    auto res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_PIP_RECT), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    ASSERT_EQ(data.WriteUint32(1), true);
    ASSERT_EQ(data.WriteInt32(1), true);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_PIP_CONTROL_STATUS), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    ASSERT_EQ(data.WriteBool(true), true);
    ASSERT_EQ(data.WriteBool(true), true);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_TITLE_AND_DOCK_HOVER_SHOW_CHANGE), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    ASSERT_EQ(data.WriteBool(true), true);
    ASSERT_EQ(data.WriteBool(false), true);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_WINDOW_RECT_AUTO_SAVE), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    ASSERT_EQ(data.WriteInt32(1), true);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_EXTENSION_EVENT_ASYNC), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    ASSERT_EQ(data.WriteFloat(1.0f), true);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_WINDOW_CORNER_RADIUS), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    ASSERT_EQ(data.WriteString("test"), true);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_FLAG), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    ASSERT_EQ(data.WriteBool(true), true);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_FOLLOW_PARENT_MULTI_SCREEN_POLICY),
        data,
        reply,
        option);
    ASSERT_EQ(ERR_NONE, res);
    ShadowsInfo shadowsInfo =
        { 20.0, "#FF0000", 0.0, 0.0, true, true, true, true }; // 20.0 is shadow radius, 0.0 is shadow offset
    ASSERT_EQ(data.WriteParcelable(&shadowsInfo), true);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_WINDOW_SHADOWS), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: ProcessRemoteRequestTest08
 * @tc.desc: sessionStub ProcessRemoteRequestTest08
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, ProcessRemoteRequestTest08, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_SYNC };
    FloatingBallTemplateInfo fbTemplateInfo {{1, "fb", "fb_content", "red"}, nullptr};
    data.WriteParcelable(&fbTemplateInfo);
    auto res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_FLOATING_BALL), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    data.WriteParcelable(nullptr);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_NOTIFY_FLOATING_BALL_PREPARE_CLOSE), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    data.WriteParcelable(nullptr);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_START_FLOATING_BALL_MAIN_WINDOW), data, reply, option);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    data.WriteParcelable(nullptr);
    res = session_->ProcessRemoteRequest(
        static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_GET_FLOATING_BALL_WINDOW_ID), data, reply, option);
    ASSERT_EQ(ERR_NONE, res);
    data.WriteParcelable(nullptr);
    res = session_->ProcessRemoteRequest(
    static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_WINDOW_ANCHOR_INFO), data, reply, option);
    ASSERT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: sessionStubTest02
 * @tc.desc: sessionStub sessionStubTest02
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, sessionStubTest02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    sptr<IRemoteObjectMocker> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_NE(data.WriteRemoteObject(iRemoteObjectMocker), false);
    ASSERT_EQ(data.WriteUint32(1), true);
    AAFwk::Want options;
    EXPECT_NE(data.WriteString("HandleSessionException"), false);
    EXPECT_NE(data.WriteParcelable(&options), false);
    ASSERT_EQ(data.WriteUint64(2), true);
    auto res = session_->HandleRaiseAboveTarget(data, reply);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->HandleRaiseAppMainWindowToTop(data, reply);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->HandleBackPressed(data, reply);
    if (!data.ReadBool()) {
        ASSERT_EQ(ERR_INVALID_DATA, res);
    }
    res = session_->HandleGetGlobalMaximizeMode(data, reply);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->HandleUpdateWindowSceneAfterCustomAnimation(data, reply);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->HandleNotifyExtensionDied(data, reply);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->HandleGetStatusBarHeight(data, reply);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: sessionStubTest03
 * @tc.desc: sessionStub sessionStubTest03
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, sessionStubTest03, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    ASSERT_EQ(data.WriteUint32(1), true);
    auto res = session_->HandleSetGlobalMaximizeMode(data, reply);
    ASSERT_EQ(ERR_NONE, res);
    res = session_->HandleTransferExtensionData(data, reply);
    ASSERT_EQ(ERR_INVALID_VALUE, res);
    ASSERT_EQ(data.WriteInt32(2), true);
    res = session_->HandleMarkProcessed(data, reply);
    ASSERT_EQ(ERR_NONE, res);
    ASSERT_EQ(data.WriteBool(true), true);
    res = session_->HandleNeedAvoid(data, reply);
    ASSERT_EQ(ERR_NONE, res);
    ASSERT_EQ(data.WriteUint32(2), true);
    ASSERT_EQ(data.WriteInt32(0), true);
    ASSERT_EQ(data.WriteInt32(0), true);
    ASSERT_EQ(data.WriteInt32(10), true);
    ASSERT_EQ(data.WriteInt32(10), true);
    ASSERT_EQ(data.WriteInt32(12), true);
    res = session_->HandleGetAvoidAreaByType(data, reply);
    ASSERT_EQ(ERR_NONE, res);
    ASSERT_EQ(data.WriteFloat(2.0f), true);
    res = session_->HandleSetAspectRatio(data, reply);
    ASSERT_EQ(ERR_NONE, res);
    ASSERT_EQ(data.WriteInt32(2), true);
    res = session_->HandleNotifyExtensionTimeout(data, reply);
    ASSERT_EQ(ERR_NONE, res);
    ASSERT_EQ(data.WriteInt32(2), true);
    AAFwk::Want want;
    data.WriteParcelable(&want);
    res = session_->HandleTransferAbilityResult(data, reply);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleTriggerBindModalUIExtension001
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleTriggerBindModalUIExtension001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    sptr<IRemoteObjectMocker> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto res = session_->HandleTriggerBindModalUIExtension(data, reply);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleTransferAccessibilityEvent002
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleTransferAccessibilityEvent003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    sptr<IRemoteObjectMocker> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto res = session_->HandleTransferAccessibilityEvent(data, reply);
    ASSERT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandleNotifyPiPWindowPrepareClose003
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleNotifyPiPWindowPrepareClose003, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    sptr<IRemoteObjectMocker> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto res = session_->HandleNotifyPiPWindowPrepareClose(data, reply);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleUpdatePiPRect001
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleUpdatePiPRect001, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    float width = 0.55;
    float height = 0.55;
    ASSERT_EQ(ERR_INVALID_DATA, session_->HandleUpdatePiPRect(data, reply));
    data.WriteInt32(-1);
    ASSERT_EQ(ERR_INVALID_DATA, session_->HandleUpdatePiPRect(data, reply));
    data.WriteInt32(10);
    data.WriteInt32(-1);
    ASSERT_EQ(ERR_INVALID_DATA, session_->HandleUpdatePiPRect(data, reply));
    data.WriteInt32(10);
    data.WriteInt32(20);
    data.WriteUint32(width);
    ASSERT_EQ(ERR_INVALID_DATA, session_->HandleUpdatePiPRect(data, reply));
    data.WriteInt32(10);
    data.WriteInt32(20);
    data.WriteUint32(30);
    data.WriteUint32(height);
    ASSERT_EQ(ERR_INVALID_DATA, session_->HandleUpdatePiPRect(data, reply));
}

/**
 * @tc.name: HandleUpdatePiPRect002
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleUpdatePiPRect002, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(10);
    data.WriteInt32(20);
    data.WriteUint32(30);
    data.WriteUint32(40);
    ASSERT_EQ(ERR_INVALID_DATA, session_->HandleUpdatePiPRect(data, reply));
    data.WriteInt32(10);
    data.WriteInt32(20);
    data.WriteUint32(30);
    data.WriteUint32(40);
    data.WriteUint32(3);
    ASSERT_EQ(ERR_NONE, session_->HandleUpdatePiPRect(data, reply));
}

/**
 * @tc.name: HandleUpdatePiPControlStatus
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleUpdatePiPControlStatus, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    uint32_t controlType = -1;
    int32_t status = 1;
    data.WriteUint32(controlType);
    data.WriteInt32(status);
    auto res = session_->HandleUpdatePiPControlStatus(data, reply);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    data.WriteUint32(10);
    data.WriteInt32(status);
    res = session_->HandleUpdatePiPControlStatus(data, reply);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    data.WriteUint32(2);
    data.WriteInt32(-4);
    res = session_->HandleUpdatePiPControlStatus(data, reply);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    data.WriteUint32(2);
    data.WriteInt32(4);
    res = session_->HandleUpdatePiPControlStatus(data, reply);
    ASSERT_EQ(ERR_INVALID_DATA, res);
    data.WriteUint32(2);
    data.WriteInt32(1);
    res = session_->HandleUpdatePiPControlStatus(data, reply);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleSetAutoStartPiP
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleSetAutoStartPiP, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    ASSERT_EQ(ERR_INVALID_DATA, session_->HandleSetAutoStartPiP(data, reply));
    bool isAutoStartValid = true;
    uint32_t priority = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    data.WriteInt32(10);
    ASSERT_EQ(ERR_INVALID_DATA, session_->HandleSetAutoStartPiP(data, reply));
    data.WriteBool(isAutoStartValid);
    data.WriteUint32(priority);
    data.WriteUint32(width);
    data.WriteUint32(height);
    ASSERT_EQ(ERR_NONE, session_->HandleSetAutoStartPiP(data, reply));
}

/**
 * @tc.name: HandleUpdatePiPTemplateInfo
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleUpdatePiPTemplateInfo, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    ASSERT_EQ(ERR_INVALID_DATA, session_->HandleUpdatePiPTemplateInfo(data, reply));
    PiPTemplateInfo templateInfo;
    data.WriteParcelable(&templateInfo);
    ASSERT_EQ(ERR_NONE, session_->HandleUpdatePiPTemplateInfo(data, reply));
}

/**
 * @tc.name: HandleSetPipParentWindowId
 * @tc.desc: sessionStub HandleSetPipParentWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleSetPipParentWindowId, Function | SmallTest | Level2)

{
    ASSERT_NE(session_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    ASSERT_EQ(ERR_INVALID_DATA, session_->HandleSetPipParentWindowId(data, reply));
    data.WriteUint32(100);
    ASSERT_EQ(ERR_NONE, session_->HandleSetPipParentWindowId(data, reply));
}

/**
 * @tc.name: HandleIsPiPActive
 * @tc.desc: sessionStub HandleIsPiPActive
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleIsPiPActive, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_IS_PIP_ACTIVE);
    data.WriteInterfaceToken(SessionStub::GetDescriptor());
    ASSERT_EQ(session_->OnRemoteRequest(code, data, reply, option), ERR_NONE);
}

/**
 * @tc.name: HandleSetWindowTransitionAnimation
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleSetWindowTransitionAnimation, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    MessageParcel data;
    MessageParcel reply;

    ASSERT_EQ(ERR_INVALID_DATA, session_->HandleSetWindowTransitionAnimation(data, reply));
    
    data.WriteInt32(10);
    ASSERT_EQ(ERR_INVALID_DATA, session_->HandleSetWindowTransitionAnimation(data, reply));
    
    TransitionAnimation animation;
    data.WriteUint32(static_cast<uint32_t>(WindowTransitionType::DESTROY));
    data.WriteParcelable(&animation);
    ASSERT_EQ(ERR_NONE, session_->HandleSetWindowTransitionAnimation(data, reply));
}

/**
 * @tc.name: HandleProcessPointDownSession006
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleProcessPointDownSession006, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteInt32(10);
    data.WriteInt32(20);
    sptr<IRemoteObjectMocker> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto res = session_->HandleProcessPointDownSession(data, reply);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleSendPointerEvenForMoveDrag007
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleSendPointerEvenForMoveDrag007, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    sptr<IRemoteObjectMocker> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    auto res = session_->HandleSendPointerEvenForMoveDrag(data, reply);
    ASSERT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandleNotifySyncOn012
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleNotifySyncOn012, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    auto res = session_->HandleNotifySyncOn(data, reply);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleNotifyAsyncOn013
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleNotifyAsyncOn013, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    auto res = session_->HandleNotifyAsyncOn(data, reply);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleNotifyExtensionTimeout014
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleNotifyExtensionTimeout014, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    auto res = session_->HandleNotifyExtensionTimeout(data, reply);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetAppForceLandscapeConfig
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleGetAppForceLandscapeConfig, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    ASSERT_NE(session_, nullptr);
    auto res = session_->HandleGetAppForceLandscapeConfig(data, reply);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleSetDialogSessionBackGestureEnabled01
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleSetDialogSessionBackGestureEnabled01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(true);
    auto res = session_->HandleSetDialogSessionBackGestureEnabled(data, reply);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleRequestFocus
 * @tc.desc: sessionStub HandleRequestFocusTest
 * @tc.type: FUNC
 * @tc.require: #IAPLFA
 */
HWTEST_F(SessionStubTest, HandleRequestFocus, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteBool(false);
    ASSERT_NE(session_, nullptr);
    auto res = session_->HandleRequestFocus(data, reply);
    ASSERT_EQ(0, res);
}

/**
 * @tc.name: HandleNotifyWindowStatusDidChangeAfterShowWindow
 * @tc.desc: sessionStub HandleNotifyWindowStatusDidChangeAfterShowWindow
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleNotifyWindowStatusDidChangeAfterShowWindow, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto result = session_->HandleNotifyWindowStatusDidChangeAfterShowWindow(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleSyncSessionEvent1
 * @tc.desc: sessionStub HandleSyncSessionEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleSyncSessionEvent1, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteUint32(static_cast<uint32_t>(SessionEvent::EVENT_MAXIMIZE));
    auto result = session_->HandleSyncSessionEvent(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleSyncSessionEvent2
 * @tc.desc: sessionStub HandleSyncSessionEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleSyncSessionEvent2, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteUint32(0xFFFFFFFF);
    auto result = session_->HandleSyncSessionEvent(data, reply);
    ASSERT_EQ(result, ERR_NONE);

    result = session_->HandleSyncSessionEvent(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: GetIsMidScene
 * @tc.desc: sessionStub GetIsMidScene
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, GetIsMidScene, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto result = session_->HandleGetIsMidScene(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleNotifyFrameLayoutFinish
 * @tc.desc: sessionStub HandleNotifyFrameLayoutFinish
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleNotifyFrameLayoutFinish, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto result = session_->HandleNotifyFrameLayoutFinish(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleRemovePrelaunchStartingWindow
 * @tc.desc: sessionStub HandleRemovePrelaunchStartingWindow
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleRemovePrelaunchStartingWindow, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto result = session_->HandleRemovePrelaunchStartingWindow(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleSnapshotUpdate
 * @tc.desc: sessionStub HandleSnapshotUpdate
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleSnapshotUpdate, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto result = session_->HandleSnapshotUpdate(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleSyncSessionEvent
 * @tc.desc: sessionStub HandleSyncSessionEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleSyncSessionEvent, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto result = session_->HandleSyncSessionEvent(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleRestoreFloatMainWindow
 * @tc.desc: sessionStub HandleRestoreFloatMainWindow
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleRestoreFloatMainWindow, TestSize.Level1)
{
    sptr<SessionStub> sessionStub = sptr<SessionStubMocker>::MakeSptr();
    ASSERT_NE(nullptr, sessionStub);
    {
        MessageParcel data1;
        MessageParcel reply1;
        EXPECT_EQ(sessionStub->HandleRestoreFloatMainWindow(data1, reply1), ERR_INVALID_VALUE);
    }
    {
        MessageParcel data2;
        MessageParcel reply2;
        std::shared_ptr<AAFwk::WantParams> wantParams = std::make_shared<AAFwk::WantParams>();
        data2.WriteParcelable(wantParams.get());
        EXPECT_EQ(sessionStub->HandleRestoreFloatMainWindow(data2, reply2), ERR_NONE);
    }
}

/**
 * @tc.name: HandlePendingSessionActivation
 * @tc.desc: sessionStub HandlePendingSessionActivation
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandlePendingSessionActivation, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto result = session_->HandlePendingSessionActivation(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: WindowCreateParams
 * @tc.desc: sessionStub WindowCreateParams
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, WindowCreateParams, TestSize.Level1)
{
    MessageParcel data;
    auto windowCreateParams = std::make_shared<WindowCreateParams>();
    windowCreateParams->animationParams = std::make_shared<StartAnimationOptions>();
    windowCreateParams->animationParams->animationType = AnimationType::FADE_IN;
    windowCreateParams->animationSystemParams = std::make_shared<StartAnimationSystemOptions>();
    data.WriteParcelable(windowCreateParams.get());
    auto windowCreateParamsRead = std::make_shared<WindowCreateParams>();
    windowCreateParamsRead.reset(data.ReadParcelable<WindowCreateParams>());
    EXPECT_EQ(windowCreateParamsRead->animationParams->animationType, AnimationType::FADE_IN);
}

/**
 * @tc.name: HandleGetGlobalScaledRect
 * @tc.desc: sessionStub HandleGetGlobalScaledRect
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleGetGlobalScaledRect, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto result = session_->HandleGetGlobalScaledRect(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleGetAllAvoidAreas
 * @tc.desc: sessionStub HandleGetAllAvoidAreas
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleGetAllAvoidAreas, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto result = session_->HandleGetAllAvoidAreas(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleIsStartMoving
 * @tc.desc: sessionStub HandleIsStartMoving
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleIsStartMoving, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto result = session_->HandleIsStartMoving(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleSetSessionLabelAndIcon01
 * @tc.desc: HandleSetSessionLabelAndIcon
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleSetSessionLabelAndIcon01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::shared_ptr<Media::PixelMap> icon = std::make_shared<Media::PixelMap>();
    data.WriteParcelable(icon.get());

    auto res = session_->HandleSetSessionLabelAndIcon(data, reply);
    ASSERT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandleSetSessionLabelAndIcon02
 * @tc.desc: HandleSetSessionLabelAndIcon
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleSetSessionLabelAndIcon02, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::string label = "demo label";
    data.WriteString(label);
    std::shared_ptr<Media::PixelMap> icon = nullptr;
    data.WriteParcelable(icon.get());

    auto res = session_->HandleSetSessionLabelAndIcon(data, reply);
    ASSERT_EQ(ERR_INVALID_DATA, res);
}

/**
 * @tc.name: HandleSetSessionLabelAndIcon03
 * @tc.desc: HandleSetSessionLabelAndIcon
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleSetSessionLabelAndIcon03, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::string label = "demo label";
    data.WriteString(label);
    const uint32_t color[] = { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 };
    uint32_t len = sizeof(color) / sizeof(color[0]);
    Media::InitializationOptions opts;
    opts.size.width = 2;
    opts.size.height = 3;
    opts.pixelFormat = Media::PixelFormat::UNKNOWN;
    opts.alphaType = Media::AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    std::shared_ptr<Media::PixelMap> icon = Media::PixelMap::Create(color, len, 0, opts.size.width, opts);
    data.WriteParcelable(icon.get());

    auto res = session_->HandleSetSessionLabelAndIcon(data, reply);
    ASSERT_EQ(ERR_NONE, res);
}

/**
 * @tc.name: HandleGetCrossAxisState
 * @tc.desc: HandleGetCrossAxisState
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleGetCrossAxisState, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    sptr<SessionStubMocker> session = sptr<SessionStubMocker>::MakeSptr();
    EXPECT_CALL(*session, GetCrossAxisState(_))
        .WillOnce(DoAll(SetArgReferee<0>(CrossAxisState::STATE_CROSS), Return(WSError::WS_OK)));
    session->HandleGetCrossAxisState(data, reply);
    uint32_t state = 0;
    reply.ReadUint32(state);
    ASSERT_EQ(state, static_cast<uint32_t>(CrossAxisState::STATE_CROSS));
}

/**
 * @tc.name: HandleContainerModalEvent
 * @tc.desc: sessionStub HandleContainerModalEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleContainerModalEvent, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteString("name");
    data.WriteString("value");
    auto result = session_->HandleContainerModalEvent(data, reply);
    ASSERT_EQ(result, ERR_NONE);
    result = session_->HandleContainerModalEvent(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleUpdateRotationChangeListenerRegistered
 * @tc.desc: sessionStub HandleUpdateRotationChangeListenerRegistered
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleUpdateRotationChangeListenerRegistered, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(0);
    data.WriteBool(false);
    auto result = session_->HandleUpdateRotationChangeListenerRegistered(data, reply);
    ASSERT_EQ(result, ERR_NONE);
    result = session_->HandleUpdateRotationChangeListenerRegistered(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleUpdateScreenshotAppEventRegistered
 * @tc.desc: sessionStub HandleUpdateScreenshotAppEventRegistered
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleUpdateScreenshotAppEventRegistered, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInt32(0);
    data.WriteBool(false);
    auto result = session_->HandleUpdateScreenshotAppEventRegistered(data, reply);
    EXPECT_EQ(result, ERR_NONE);

    data.WriteInterfaceToken(u"OHOS.ISession");
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_SCREEN_SHOT_APP_EVENT_REGISTERED);
    EXPECT_EQ(0, session_->OnRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateAcrossDisplaysChangeRegistered
 * @tc.desc: sessionStub HandleUpdateAcrossDisplaysChangeRegistered
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleUpdateAcrossDisplaysChangeRegistered, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInt32(0);
    data.WriteBool(false);
    sptr<SessionStub> sessionStub = sptr<SessionStubMocker>::MakeSptr();
    ASSERT_NE(nullptr, sessionStub);
    auto result = sessionStub->HandleUpdateAcrossDisplaysChangeRegistered(data, reply);
    EXPECT_EQ(result, ERR_NONE);

    data.WriteInterfaceToken(SessionStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_ACROSS_DISPLAYS_REGISTERED);
    EXPECT_EQ(0, sessionStub->ProcessRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleIsMainWindowFullScreenAcrossDisplays
 * @tc.desc: sessionStub HandleIsMainWindowFullScreenAcrossDisplays
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleIsMainWindowFullScreenAcrossDisplays, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInt32(0);
    data.WriteBool(false);
    sptr<SessionStub> sessionStub = sptr<SessionStubMocker>::MakeSptr();
    ASSERT_NE(nullptr, sessionStub);
    auto result = sessionStub->HandleIsMainWindowFullScreenAcrossDisplays(data, reply);
    EXPECT_EQ(result, ERR_NONE);

    data.WriteInterfaceToken(SessionStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_MAIN_WINDOW_FULL_SCREEN_ACROSS_DISPLAYS);
    EXPECT_EQ(0, sessionStub->ProcessRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateColorMode
 * @tc.desc: sessionStub HandleUpdateColorMode
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleUpdateColorMode, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteString("DARK");
    data.WriteBool(true);
    sptr<SessionStub> sessionStub = sptr<SessionStubMocker>::MakeSptr();
    ASSERT_NE(nullptr, sessionStub);
    auto result = sessionStub->HandleUpdateColorMode(data, reply);
    EXPECT_EQ(result, ERR_NONE);

    data.WriteInterfaceToken(SessionStub::GetDescriptor());
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_COLOR_MODE);
    EXPECT_EQ(0, sessionStub->ProcessRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleNotifyKeyboardWillShowRegistered
 * @tc.desc: sessionStub HandleNotifyKeyboardWillShowRegistered
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleNotifyKeyboardWillShowRegistered, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleNotifyKeyboardWillShowRegistered(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);

    data.WriteBool(true);
    auto result1 = session_->HandleNotifyKeyboardWillShowRegistered(data, reply);
    ASSERT_EQ(result1, ERR_NONE);
}

/**
 * @tc.name: HandleNotifyKeyboardWillHideRegistered
 * @tc.desc: sessionStub HandleNotifyKeyboardWillHideRegistered
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleNotifyKeyboardWillHideRegistered, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleNotifyKeyboardWillHideRegistered(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
 
    data.WriteBool(true);
    auto result1 = session_->HandleNotifyKeyboardWillHideRegistered(data, reply);
    ASSERT_EQ(result1, ERR_NONE);
}

/**
 * @tc.name: HandleGetTargetOrientationConfigInfo
 * @tc.desc: sessionStub HandleGetTargetOrientationConfigInfo
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleGetTargetOrientationConfigInfo, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteUint32(-1);
    data.WriteBool(true);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteBool(true);
    data.WriteUint32(1);
    auto result = session_->HandleGetTargetOrientationConfigInfo(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteBool(true);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteBool(true);
    data.WriteUint32(1);
    
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteBool(true);
    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteBool(true);
    data.WriteUint32(1);
    auto result1 = session_->HandleGetTargetOrientationConfigInfo(data, reply);
    EXPECT_EQ(result1, ERR_NONE);
}

/**
 * @tc.name: HandleConvertOrientationAndRotationWithInvalidFrom
 * @tc.desc: Verify HandleConvertOrientationAndRotation with invalid from
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleConvertOrientationAndRotationWithInvalidFrom, TestSize.Level1)
{
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_CONVERT_ORIENTATION_AND_ROTATION);
    MessageParcel reply;
    MessageOption option;
    MessageParcel data;
    // missing FromRotationInfoType from
    EXPECT_EQ(ERR_INVALID_DATA, session_->ProcessRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleConvertOrientationAndRotationWithInvalidTo
 * @tc.desc: Verify  HandleConvertOrientationAndRotation with invalid to
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleConvertOrientationAndRotationWithInvalidTo, TestSize.Level1)
{
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_CONVERT_ORIENTATION_AND_ROTATION);
    MessageParcel reply;
    MessageOption option;
    MessageParcel data;
    // missing FromRotationInfoType to
    data.WriteUint32(0);
    EXPECT_EQ(ERR_INVALID_DATA, session_->ProcessRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name:  HandleConvertOrientationAndRotationWithInvalidConvertValue
 * @tc.desc: Verify  HandleConvertOrientationAndRotation with invalid convert value
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest,  HandleConvertOrientationAndRotationWithInvalidConvertValue, TestSize.Level1)
{
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_CONVERT_ORIENTATION_AND_ROTATION);
    MessageParcel reply;
    MessageOption option;
    MessageParcel data;
    // missing convert value
    data.WriteUint32(0);
    data.WriteUint32(0);
    EXPECT_EQ(ERR_INVALID_DATA, session_->ProcessRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name:  HandleConvertOrientationAndRotationWithInvalidRotationInfoType
 * @tc.desc: Verify that ProcessRemoteRequest rejects invalid RotationInfoType values
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleConvertOrientationAndRotationWithInvalidRotationInfoType, TestSize.Level1)
{
    constexpr uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_CONVERT_ORIENTATION_AND_ROTATION);
    MessageOption option;

    // Case 1: from < RotationInfoType::WINDOW_ORIENTATION
    {
        MessageParcel data;
        MessageParcel reply;
        data.WriteUint32(static_cast<uint32_t>(RotationInfoType::WINDOW_ORIENTATION) - 1);
        data.WriteUint32(0);
        data.WriteInt32(0);
        EXPECT_EQ(session_->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    }

    // Case 2: from > RotationInfoType::DISPLAY_ROTATION
    {
        MessageParcel data;
        MessageParcel reply;
        data.WriteUint32(static_cast<uint32_t>(RotationInfoType::DISPLAY_ROTATION) + 1);
        data.WriteUint32(0);
        data.WriteInt32(0);
        EXPECT_EQ(session_->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    }

    // Case 3: to < RotationInfoType::WINDOW_ORIENTATION
    {
        MessageParcel data;
        MessageParcel reply;
        data.WriteUint32(0);
        data.WriteUint32(static_cast<uint32_t>(RotationInfoType::WINDOW_ORIENTATION) - 1);
        data.WriteInt32(0);
        EXPECT_EQ(session_->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    }

    // Case 4: to > RotationInfoType::DISPLAY_ROTATION
    {
        MessageParcel data;
        MessageParcel reply;
        data.WriteUint32(0);
        data.WriteUint32(static_cast<uint32_t>(RotationInfoType::DISPLAY_ROTATION) + 1);
        data.WriteInt32(0);
        EXPECT_EQ(session_->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    }
}

/**
 * @tc.name: HandleConvertOrientationAndRotationSuccess
 * @tc.desc: Verify that ProcessRemoteRequest accepts Success
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleConvertOrientationAndRotationSuccess, TestSize.Level1)
{
    constexpr uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_CONVERT_ORIENTATION_AND_ROTATION);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteUint32(1);
    data.WriteUint32(1);
    data.WriteInt32(1);

    EXPECT_EQ(session_->ProcessRemoteRequest(code, data, reply, option), ERR_NONE);
}

/**
 * @tc.name: GetIsHighlighted
 * @tc.desc: sessionStub GetIsHighlighted
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, GetIsHighlighted, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleGetIsHighlighted(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleDefaultDensityEnabled
 * @tc.desc: sessionStub HandleDefaultDensityEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleDefaultDensityEnabled, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleDefaultDensityEnabled(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
    data.WriteBool(true);
    result = session_->HandleDefaultDensityEnabled(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleForeground
 * @tc.desc: sessionStub HandleForeground
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleForeground, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleForeground(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
    data.WriteBool(true);
    sptr<WindowSessionProperty> propertyTest = nullptr;
    data.WriteParcelable(propertyTest);
    result = session_->HandleForeground(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
    propertyTest = sptr<WindowSessionProperty>::MakeSptr();
    data.WriteBool(true);
    data.WriteParcelable(propertyTest);
    data.WriteBool(true);
    result = session_->HandleForeground(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
    data.WriteBool(true);
    data.WriteParcelable(propertyTest);
    data.WriteBool(true);
    data.WriteString("HandleForegroundTest");
    result = session_->HandleForeground(data, reply);
    ASSERT_EQ(result, ERR_NONE);
    data.WriteBool(false);
    data.WriteBool(true);
    data.WriteString("HandleForegroundTest");
    result = session_->HandleForeground(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleGetWaterfallMode
 * @tc.desc: sessionStub HandleGetWaterfallMode
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleGetWaterfallMode, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleGetWaterfallMode(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleKeyFrameAnimateEnd
 * @tc.desc: sessionStub HandleKeyFrameAnimateEnd
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleKeyFrameAnimateEnd, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleKeyFrameAnimateEnd(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleUpdateKeyFrameCloneNode
 * @tc.desc: sessionStub HandleUpdateKeyFrameCloneNode
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleUpdateKeyFrameCloneNode, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto rsKeyFrameNode = RSWindowKeyFrameNode::Create();
    ASSERT_NE(rsKeyFrameNode, nullptr);
    ASSERT_EQ(rsKeyFrameNode->WriteToParcel(data), true);
    auto rsTransaction = std::make_shared<RSTransaction>();
    ASSERT_NE(rsTransaction, nullptr);
    ASSERT_EQ(data.WriteParcelable(rsTransaction.get()), true);
    auto result = session_->HandleUpdateKeyFrameCloneNode(data, reply);
    ASSERT_EQ(result, ERR_NONE);
    int32_t ret = 0;
    ASSERT_EQ(reply.ReadInt32(ret), true);
}

/**
 * @tc.name: HandleNotifyDisableDelegatorChange
 * @tc.desc: sessionStub HandleNotifyDisableDelegatorChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleNotifyDisableDelegatorChange, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleNotifyDisableDelegatorChange(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleNotifyKeyboardDidHideRegistered
 * @tc.desc: sessionStub HandleNotifyKeyboardDidHideRegistered
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleNotifyKeyboardDidHideRegistered, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleNotifyKeyboardDidHideRegistered(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
    data.WriteBool(true);
    result = session_->HandleNotifyKeyboardDidHideRegistered(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleNotifyKeyboardDidShowRegistered
 * @tc.desc: sessionStub HandleNotifyKeyboardDidShowRegistered
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleNotifyKeyboardDidShowRegistered, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleNotifyKeyboardDidShowRegistered(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
    data.WriteBool(true);
    result = session_->HandleNotifyKeyboardDidShowRegistered(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleNotifyMainModalTypeChange
 * @tc.desc: sessionStub HandleNotifyMainModalTypeChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleNotifyMainModalTypeChange, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleNotifyMainModalTypeChange(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
    data.WriteBool(true);
    result = session_->HandleNotifyMainModalTypeChange(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleNotifySubModalTypeChange
 * @tc.desc: sessionStub HandleNotifySubModalTypeChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleNotifySubModalTypeChange, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleNotifySubModalTypeChange(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
    data.WriteUint32(static_cast<uint32_t>(SubWindowModalType::TYPE_UNDEFINED));
    result = session_->HandleNotifySubModalTypeChange(data, reply);
    ASSERT_EQ(result, ERR_NONE);
    data.WriteUint32(static_cast<uint32_t>(SubWindowModalType::TYPE_TEXT_MENU) + 1);
    result = session_->HandleNotifySubModalTypeChange(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleSetFollowParentWindowLayoutEnabled
 * @tc.desc: sessionStub HandleSetFollowParentWindowLayoutEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleSetFollowParentWindowLayoutEnabled, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleSetFollowParentWindowLayoutEnabled(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
    data.WriteBool(true);
    result = session_->HandleSetFollowParentWindowLayoutEnabled(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleSetWindowAnchorInfo
 * @tc.desc: sessionStub HandleSetWindowAnchorInfo
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleSetWindowAnchorInfo, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleSetWindowAnchorInfo(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);

    WindowAnchorInfo windowAnchor;
    data.WriteParcelable(&windowAnchor);
    result = session_->HandleSetWindowAnchorInfo(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleSetGestureBackEnabled
 * @tc.desc: sessionStub HandleSetGestureBackEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleSetGestureBackEnabled, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleSetGestureBackEnabled(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
    data.WriteBool(true);
    result = session_->HandleSetGestureBackEnabled(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleSetSupportedWindowModes
 * @tc.desc: sessionStub HandleSetSupportedWindowModes
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleSetSupportedWindowModes, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    std::vector<AppExecFwk::SupportWindowMode> supportedWindowModes = { AppExecFwk::SupportWindowMode::FULLSCREEN,
                                                                        AppExecFwk::SupportWindowMode::SPLIT,
                                                                        AppExecFwk::SupportWindowMode::FLOATING };
    auto result = session_->HandleSetSupportedWindowModes(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);
    uint32_t size = supportedWindowModes.size();
    data.WriteUint32(size);
    for (int i = 0; i < size; i++) {
        data.WriteInt32(static_cast<int32_t>(supportedWindowModes[i]));
    }
    result = session_->HandleSetSupportedWindowModes(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleStartMovingWithCoordinate
 * @tc.desc: sessionStub HandleStartMovingWithCoordinate
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleStartMovingWithCoordinate, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleStartMovingWithCoordinate(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);
    data.WriteInt32(0);
    result = session_->HandleStartMovingWithCoordinate(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);
    data.WriteInt32(0);
    data.WriteInt32(0);
    result = session_->HandleStartMovingWithCoordinate(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);
    data.WriteInt32(0);
    data.WriteInt32(0);
    data.WriteInt32(1);
    result = session_->HandleStartMovingWithCoordinate(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);
    data.WriteInt32(0);
    data.WriteInt32(0);
    data.WriteInt32(1);
    data.WriteInt32(1);
    result = session_->HandleStartMovingWithCoordinate(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);
    data.WriteInt32(0);
    data.WriteInt32(0);
    data.WriteInt32(1);
    data.WriteInt32(1);
    data.WriteUint64(0);
    result = session_->HandleStartMovingWithCoordinate(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**q
 * @tc.name: HandleTransferAccessibilityEvent
 * @tc.desc: sessionStub HandleTransferAccessibilityEvent
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleTransferAccessibilityEvent, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteParcelable(nullptr);
    auto result = session_->HandleTransferAccessibilityEvent(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
    Accessibility::AccessibilityEventInfo info;
    Accessibility::AccessibilityEventInfoParcel infoParcel(info);
    data.WriteParcelable(&infoParcel);
    result = session_->HandleTransferAccessibilityEvent(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
    data.WriteParcelable(&infoParcel);
    data.WriteInt64(1);
    result = session_->HandleTransferAccessibilityEvent(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleSetSubWindowSource
 * @tc.desc: test function : HandleSetSubWindowSource
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleSetSubWindowSource, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    Accessibility::AccessibilityEventInfo info;
    Accessibility::AccessibilityEventInfoParcel infoParcel(info);
    auto result = session_->HandleSetSubWindowSource(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);
    data.WriteParcelable(&infoParcel);
    data.WriteUint32(1);
    result = session_->HandleSetSubWindowSource(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleChangeKeyboardEffectOption01
 * @tc.desc: test function : HandleChangeKeyboardEffectOption
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleChangeKeyboardEffectOption01, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    auto result = session_->HandleChangeKeyboardEffectOption(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);

    KeyboardEffectOption effectOption;
    data.WriteParcelable(&effectOption);
    result = session_->HandleChangeKeyboardEffectOption(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleChangeKeyboardEffectOption02
 * @tc.desc: test function : HandleChangeKeyboardEffectOption
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleChangeKeyboardEffectOption02, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = static_cast<KeyboardViewMode>(MAX_INT);
    data.WriteParcelable(&effectOption);
    auto result = session_->HandleChangeKeyboardEffectOption(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleChangeKeyboardEffectOption03
 * @tc.desc: test function : HandleChangeKeyboardEffectOption
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleChangeKeyboardEffectOption03, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    KeyboardEffectOption effectOption;
    effectOption.flowLightMode_ = static_cast<KeyboardFlowLightMode>(MAX_INT);
    data.WriteParcelable(&effectOption);
    auto result = session_->HandleChangeKeyboardEffectOption(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleChangeKeyboardEffectOption04
 * @tc.desc: test function : HandleChangeKeyboardEffectOption
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleChangeKeyboardEffectOption04, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    KeyboardEffectOption effectOption;
    effectOption.gradientMode_ = static_cast<KeyboardGradientMode>(MAX_INT);
    data.WriteParcelable(&effectOption);
    auto result = session_->HandleChangeKeyboardEffectOption(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleSetFrameRectForPartialZoomIn
 * @tc.desc: sessionStub sessionStubTest
 * @tc.type: FUNC
 * @tc.require: #I6JLSI
 */
HWTEST_F(SessionStubTest, HandleSetFrameRectForPartialZoomIn, Function | SmallTest | Level2)
{
    EXPECT_NE(session_, nullptr);
    MessageParcel data;
    MessageParcel reply;

    EXPECT_EQ(session_->HandleSetFrameRectForPartialZoomIn(data, reply), ERR_INVALID_DATA);
    
    bool res = data.WriteUint32(0);
    EXPECT_EQ(res, true);
    EXPECT_EQ(session_->HandleSetFrameRectForPartialZoomIn(data, reply), ERR_INVALID_DATA);

    res = data.WriteInt32(0) && data.WriteUint32(0);
    EXPECT_EQ(res, true);
    EXPECT_EQ(session_->HandleSetFrameRectForPartialZoomIn(data, reply), ERR_INVALID_DATA);

    res = data.WriteInt32(0) && data.WriteInt32(0) && data.WriteInt32(0);
    EXPECT_EQ(res, true);
    EXPECT_EQ(session_->HandleSetFrameRectForPartialZoomIn(data, reply), ERR_INVALID_DATA);
    
    res = data.WriteInt32(0) && data.WriteInt32(0) && data.WriteUint32(0) && data.WriteUint32(0);
    EXPECT_EQ(res, true);
    EXPECT_EQ(session_->HandleSetFrameRectForPartialZoomIn(data, reply), ERR_NONE);
}

/**
 * @tc.name: HandleUpdateFloatingBall
 * @tc.desc: sessionStub HandleUpdateFloatingBall
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleUpdateFloatingBall, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteParcelable(nullptr);
    auto result = session_->HandleUpdateFloatingBall(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
 
    FloatingBallTemplateInfo fbTemplateInfo {{1, "fb", "fb_content", "red"}, nullptr};
    data.WriteParcelable(&fbTemplateInfo);
    result = session_->HandleUpdateFloatingBall(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleStopFloatingBall
 * @tc.desc: sessionStub HandleStopFloatingBall
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleStopFloatingBall, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteParcelable(nullptr);
    auto result = session_->HandleStopFloatingBall(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleStartFloatingBallMainWindow
 * @tc.desc: sessionStub HandleStartFloatingBallMainWindow
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleStartFloatingBallMainWindow, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    
    data.WriteParcelable(nullptr);
    auto result = session_->HandleStartFloatingBallMainWindow(data, reply);
    ASSERT_EQ(result, ERR_INVALID_DATA);
 
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    data.WriteParcelable(want.get());
    result = session_->HandleStartFloatingBallMainWindow(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleGetFloatingBallWindowId
 * @tc.desc: sessionStub HandleGetFloatingBallWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleGetFloatingBallWindowId, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;
    data.WriteParcelable(nullptr);
    auto result = session_->HandleGetFloatingBallWindowId(data, reply);
    ASSERT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleUpdateGlobalDisplayRectFromClientWithInvalidRect
 * @tc.desc: Verify HandleUpdateGlobalDisplayRectFromClient with invalid rect
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleUpdateGlobalDisplayRectFromClientWithInvalidRect, TestSize.Level1)
{
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_GLOBAL_DISPLAY_RECT);
    MessageParcel reply;
    MessageOption option;
    MessageParcel data;
    data.WriteInt32(100); // posX
    data.WriteInt32(200); // posY
    data.WriteInt32(300); // width
    // missing height
    EXPECT_EQ(ERR_INVALID_DATA, session_->ProcessRemoteRequest(code, data, reply, option));
}

/**
 * @tc.name: HandleUpdateGlobalDisplayRectFromClientWithInvalidReason
 * @tc.desc: Verify that ProcessRemoteRequest rejects missing or invalid reason values
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleUpdateGlobalDisplayRectFromClientWithInvalidReason, TestSize.Level1)
{
    constexpr uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_GLOBAL_DISPLAY_RECT);
    MessageOption option;

    auto writeRect = [](MessageParcel& data) {
        data.WriteInt32(10); // posX
        data.WriteInt32(20); // posY
        data.WriteInt32(300); // width
        data.WriteInt32(400); // height
    };

    // Case 1: Missing reason field
    {
        MessageParcel data;
        MessageParcel reply;
        writeRect(data);
        EXPECT_EQ(session_->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    }

    // Case 2: reason < SizeChangeReason::UNDEFINED
    {
        MessageParcel data;
        MessageParcel reply;
        writeRect(data);
        data.WriteUint32(static_cast<uint32_t>(SizeChangeReason::UNDEFINED) - 1);
        EXPECT_EQ(session_->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    }

    // Case 3: reason == SizeChangeReason::END (boundary overflow)
    {
        MessageParcel data;
        MessageParcel reply;
        writeRect(data);
        data.WriteUint32(static_cast<uint32_t>(SizeChangeReason::END));
        EXPECT_EQ(session_->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    }

    // Case 4: reason > SizeChangeReason::END
    {
        MessageParcel data;
        MessageParcel reply;
        writeRect(data);
        data.WriteUint32(static_cast<uint32_t>(SizeChangeReason::END) + 1);
        EXPECT_EQ(session_->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    }
}

/**
 * @tc.name: HandleUpdateGlobalDisplayRectFromClientSuccess
 * @tc.desc: Verify that ProcessRemoteRequest accepts valid rect and reason data
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleUpdateGlobalDisplayRectFromClientSuccess, TestSize.Level1)
{
    constexpr int32_t posX = 100;
    constexpr int32_t posY = 200;
    constexpr int32_t width = 300;
    constexpr int32_t height = 400;
    constexpr SizeChangeReason reason = SizeChangeReason::RESIZE;
    constexpr uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_UPDATE_GLOBAL_DISPLAY_RECT);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInt32(posX);
    data.WriteInt32(posY);
    data.WriteInt32(width);
    data.WriteInt32(height);
    data.WriteUint32(static_cast<uint32_t>(reason));

    EXPECT_EQ(session_->ProcessRemoteRequest(code, data, reply, option), ERR_NONE);
}

/**
 * @tc.name: HandleConnect001
 * @tc.desc: sessionStub HandleConnect001
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleConnect001, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    auto result = session_->HandleConnect(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    data.WriteRemoteObject(sessionStage->AsObject());
    result = session_->HandleConnect(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);

    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    result = session_->HandleConnect(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);

    std::string surfaceNodeName = "HandleConnectTest";
    Rosen::RSSurfaceNodeConfig surfaceNodeConfig = { .SurfaceNodeName = surfaceNodeName};
    std::shared_ptr<Rosen::RSSurfaceNode> rsSurfaceNode;
    rsSurfaceNode = std::make_shared<Rosen::RSSurfaceNode>(surfaceNodeConfig, true);
    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    rsSurfaceNode->Marshalling(data);
    result = session_->HandleConnect(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);

    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    rsSurfaceNode->Marshalling(data);
    data.WriteBool(false);
    result = session_->HandleConnect(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleConnect002
 * @tc.desc: sessionStub HandleConnect002
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleConnect002, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    Rosen::RSSurfaceNodeConfig surfaceNodeConfig = { .SurfaceNodeName = "HandleConnectTest"};
    std::shared_ptr<Rosen::RSSurfaceNode> rsSurfaceNode;
    rsSurfaceNode = std::make_shared<Rosen::RSSurfaceNode>(surfaceNodeConfig, true);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetTokenState(true);
    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    rsSurfaceNode->Marshalling(data);
    data.WriteBool(true);
    auto result = session_->HandleConnect(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);

    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    rsSurfaceNode->Marshalling(data);
    data.WriteBool(true);
    data.WriteParcelable(property.GetRefPtr());
    result = session_->HandleConnect(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);

    sptr<IRemoteObject> token = nullptr;
    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    rsSurfaceNode->Marshalling(data);
    data.WriteBool(true);
    data.WriteParcelable(property.GetRefPtr());
    data.WriteRemoteObject(token);
    result = session_->HandleConnect(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleConnect003
 * @tc.desc: sessionStub HandleConnect003
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleConnect003, Function | SmallTest | Level2)
{
    MessageParcel data;
    MessageParcel reply;

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    Rosen::RSSurfaceNodeConfig surfaceNodeConfig = { .SurfaceNodeName = "HandleConnectTest"};
    std::shared_ptr<Rosen::RSSurfaceNode> rsSurfaceNode;
    rsSurfaceNode = std::make_shared<Rosen::RSSurfaceNode>(surfaceNodeConfig, true);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetTokenState(true);

    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    rsSurfaceNode->Marshalling(data);
    data.WriteBool(true);
    data.WriteParcelable(property.GetRefPtr());
    data.WriteRemoteObject(token);
    auto result = session_->HandleConnect(data, reply);
    EXPECT_EQ(result, ERR_INVALID_DATA);

    std::string identityToken = "HandleConnectTest";
    data.WriteRemoteObject(sessionStage->AsObject());
    data.WriteRemoteObject(eventChannel->AsObject());
    rsSurfaceNode->Marshalling(data);
    data.WriteBool(true);
    data.WriteParcelable(property.GetRefPtr());
    data.WriteRemoteObject(token);
    data.WriteString(identityToken);
    result = session_->HandleConnect(data, reply);
    EXPECT_EQ(result, ERR_NONE);
}

/**
 * @tc.name: HandleSetContentAspectRatioCases
 * @tc.desc: Verify HandleSetContentAspectRatio with invalid and valid inputs
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleSetContentAspectRatioCases, TestSize.Level1)
{
    sptr<SessionStubMocker> session = sptr<SessionStubMocker>::MakeSptr();
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_CONTENT_ASPECT_RATIO);
    MessageOption option;
    float ratio = 1.5f;
    bool isPersistent = true;
    bool needUpdateRect = false;

    // Case 1: Missing ratio
    {
        MessageParcel data;
        MessageParcel reply;
        EXPECT_EQ(session->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    }

    // Case 2: Missing isPersistent and needUpdateRect
    {
        MessageParcel data;
        MessageParcel reply;
        data.WriteFloat(ratio);
        EXPECT_EQ(session->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    }

    // Case 3: Missing needUpdateRect
    {
        MessageParcel data;
        MessageParcel reply;
        data.WriteFloat(ratio);
        data.WriteBool(isPersistent);
        EXPECT_EQ(session->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    }

    // Case 4: Success
    {
        MessageParcel data;
        MessageParcel reply;
        data.WriteFloat(ratio);
        data.WriteBool(isPersistent);
        data.WriteBool(needUpdateRect);
        EXPECT_CALL(*session, SetContentAspectRatio(ratio, isPersistent, needUpdateRect)).Times(1);
        EXPECT_EQ(session->ProcessRemoteRequest(code, data, reply, option), ERR_NONE);
    }
}

/**
 * @tc.name: HandleSetDecorVisibleCases
 * @tc.desc: Verify HandleSetDecorVisible with invalid and valid inputs
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleSetDecorVisibleCases, TestSize.Level1)
{
    sptr<SessionStubMocker> session = sptr<SessionStubMocker>::MakeSptr();
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SET_DECOR_VISIBLE);
    MessageOption option;

    // Case 1: Missing isVisible
    {
        MessageParcel data;
        MessageParcel reply;
        EXPECT_EQ(session->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    }

    // Case 2: Success
    {
        MessageParcel data;
        MessageParcel reply;
        bool isVisible = true;
        data.WriteBool(isVisible);
        EXPECT_CALL(*session, SetDecorVisible(isVisible)).Times(1);
        EXPECT_EQ(session->ProcessRemoteRequest(code, data, reply, option), ERR_NONE);
    }
}

/**
 * @tc.name: HandleNotifyIsFullScreenInForceSplitMode
 * @tc.desc: HandleNotifyIsFullScreenInForceSplitMode test
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleNotifyIsFullScreenInForceSplitMode, TestSize.Level1)
{
    {
        MessageOption option;
        MessageParcel data;
        MessageParcel reply;
        MockMessageParcel::SetReadBoolErrorFlag(true);
        EXPECT_EQ(session->HandleNotifyIsFullScreenInForceSplitMode(data, reply), ERR_INVALID_DATA);
        MockMessageParcel::SetReadBoolErrorFlag(false);
    }
    {
        MessageOption option;
        MessageParcel data;
        MessageParcel reply;
        bool isFullScreen = true;
        data.WriteBool(isFullScreen);
        MockMessageParcel::SetWriteInt32ErrorFlag(true);
        EXPECT_EQ(session->HandleNotifyIsFullScreenInForceSplitMode(data, reply), ERR_INVALID_DATA);
        MockMessageParcel::SetWriteInt32ErrorFlag(false);
    }
    {
        MessageOption option;
        MessageParcel data;
        MessageParcel reply;
        bool isFullScreen = true;
        data.WriteBool(isFullScreen);
        EXPECT_EQ(session->HandleNotifyIsFullScreenInForceSplitMode(code, data, reply, option), ERR_NONE);
    }
}

/**
 * @tc.name: HandleRestartApp
 * @tc.desc: HandleRestartApp test
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, HandleRestartApp, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    data.WriteParcelable(nullptr);
    ASSERT_EQ(session_->HandleRestartApp(data, reply), ERR_INVALID_DATA);

    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    data.WriteParcelable(want.get());
    ASSERT_EQ(session_->HandleRestartApp(data, reply), ERR_NONE);
}

/**
 * @tc.name: TestHandleSessionEventWithInvalidInputs
 * @tc.desc: Verify that HandleSessionEvent correctly rejects invalid input data.
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, TestHandleSessionEventWithInvalidInputs, TestSize.Level1)
{
    sptr<SessionStubMocker> session = sptr<SessionStubMocker>::MakeSptr();
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SESSION_EVENT);
    MessageOption option;

    // Case 1: Missing eventId → should fail before invoking OnSessionEvent()
    {
        MessageParcel data;
        MessageParcel reply;
        EXPECT_CALL(*session, OnSessionEvent(_, _)).Times(0);
        EXPECT_EQ(session->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    }

    // Case 2: eventId below EVENT_MAXIMIZE → treated as invalid
    {
        MessageParcel data;
        MessageParcel reply;
        data.WriteUint32(static_cast<uint32_t>(SessionEvent::EVENT_MAXIMIZE) - 1);
        EXPECT_CALL(*session, OnSessionEvent(_, _)).Times(0);
        EXPECT_EQ(session->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    }

    // Case 3: eventId equal or beyond EVENT_END → treated as invalid
    {
        MessageParcel data;
        MessageParcel reply;
        data.WriteUint32(static_cast<uint32_t>(SessionEvent::EVENT_END));
        EXPECT_CALL(*session, OnSessionEvent(_, _)).Times(0);
        EXPECT_EQ(session->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    }

    // Case 4: EVENT_MAXIMIZE but missing 'waterfallResidentState' parameter → should fail
    {
        MessageParcel data;
        MessageParcel reply;
        data.WriteUint32(static_cast<uint32_t>(SessionEvent::EVENT_MAXIMIZE));
        EXPECT_CALL(*session, OnSessionEvent(_, _)).Times(0);
        EXPECT_EQ(session->ProcessRemoteRequest(code, data, reply, option), ERR_INVALID_DATA);
    }
}

/**
 * @tc.name: TestHandleSessionEventWithValidInputs
 * @tc.desc: Verify that HandleSessionEvent correctly processes valid input data and writes proper response.
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, TestHandleSessionEventWithValidInputs, TestSize.Level1)
{
    sptr<SessionStubMocker> session = sptr<SessionStubMocker>::MakeSptr();
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SESSION_EVENT);
    MessageOption option;

    // Case 1: EVENT_MAXIMIZE with valid 'waterfallResidentState' → should succeed and return WS_OK
    {
        MessageParcel data;
        MessageParcel reply;
        uint32_t eventId = static_cast<uint32_t>(SessionEvent::EVENT_MAXIMIZE);
        uint32_t waterfallResidentState = 0;
        data.WriteUint32(eventId);
        data.WriteUint32(waterfallResidentState);

        EXPECT_CALL(*session, OnSessionEvent(_, _))
            .Times(1)
            .WillOnce(testing::Return(WSError::WS_OK));

        int ret = session->ProcessRemoteRequest(code, data, reply, option);
        EXPECT_EQ(ret, ERR_NONE);

        uint32_t errCode = reply.ReadUint32();
        EXPECT_EQ(errCode, static_cast<uint32_t>(WSError::WS_OK));
    }

    // Case 2: Non-EVENT_MAXIMIZE event (e.g. EVENT_MINIMIZE) → should succeed without reading extra params
    {
        MessageParcel data;
        MessageParcel reply;
        uint32_t eventId = static_cast<uint32_t>(SessionEvent::EVENT_MINIMIZE);
        uint32_t waterfallResidentState = 1;
        data.WriteUint32(eventId);

        EXPECT_CALL(*session, OnSessionEvent(_, _))
            .Times(1)
            .WillOnce(testing::Return(WSError::WS_OK));

        int ret = session->ProcessRemoteRequest(code, data, reply, option);
        EXPECT_EQ(ret, ERR_NONE);

        uint32_t errCode = reply.ReadUint32();
        EXPECT_EQ(errCode, static_cast<uint32_t>(WSError::WS_OK));
    }
}
/**
 * @tc.name: TestHandleSessionEventWithValidInputs
 * @tc.desc: Verify that HandleSessionEvent correctly processes valid input data and writes proper response.
 * @tc.type: FUNC
 */
HWTEST_F(SessionStubTest, TestHandleSessionEventWithValidInputs02, TestSize.Level1)
{
    sptr<SessionStubMocker> session = sptr<SessionStubMocker>::MakeSptr();
    uint32_t code = static_cast<uint32_t>(SessionInterfaceCode::TRANS_ID_SESSION_EVENT);
    MessageOption option;
 
    // Case 1: EVENT_MAXIMIZE with valid 'compatibleStyleMode' → should succeed and return WS_OK
    {
        MessageParcel data;
        MessageParcel reply;
        uint32_t eventId = static_cast<uint32_t>(SessionEvent::EVENT_SWITCH_COMPATIBLE_MODE);
        uint32_t compatibleStyleMode = 0;
        data.WriteUint32(eventId);
        data.WriteUint32(compatibleStyleMode);
 
        EXPECT_CALL(*session, OnSessionEvent(_, _))
            .Times(1)
            .WillOnce(testing::Return(WSError::WS_OK));
 
        int ret = session->ProcessRemoteRequest(code, data, reply, option);
        EXPECT_EQ(ret, ERR_NONE);
 
        uint32_t errCode = reply.ReadUint32();
        EXPECT_EQ(errCode, static_cast<uint32_t>(WSError::WS_OK));
    }
}
} // namespace
} // namespace Rosen
} // namespace OHOS