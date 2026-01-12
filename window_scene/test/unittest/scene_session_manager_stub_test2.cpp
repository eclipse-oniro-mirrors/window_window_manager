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
#include <message_option.h>
#include <message_parcel.h>

#include "iremote_object_mocker.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_manager/include/zidl/scene_session_manager_interface.h"
#include "session/container/include/window_event_channel.h"
#include "window_manager_agent.h"
#include "zidl/scene_session_manager_stub.h"
#include "zidl/window_manager_agent_interface.h"
#include "pattern_detach_callback.h"
#include "test/mock/mock_session_stage.h"
#include "mock/mock_message_parcel.h"
#include "test/mock/mock_scene_session_manager_stub.h"
#include "ui_effect_controller_client.h"
#include "ui_effect_controller.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class SceneSessionManagerStubTest2 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<SceneSessionManagerStub> stub_;
private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void SceneSessionManagerStubTest2::SetUpTestCase()
{
}

void SceneSessionManagerStubTest2::TearDownTestCase()
{
}

void SceneSessionManagerStubTest2::SetUp()
{
    stub_ = sptr<SceneSessionManager>::MakeSptr();
}

void SceneSessionManagerStubTest2::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
}

namespace {

/**
 * @tc.name: HandleIsFreeMultiWindow
 * @tc.desc: test HandleIsFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest2, HandleIsFreeMultiWindow, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    
    // writeInt32 failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    int res = stub_->HandleIsFreeMultiWindow(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
    MockMessageParcel::ClearAllErrorFlag();
    
    // writeBool failed
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    res = stub_->HandleIsFreeMultiWindow(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    // interface success
    MockMessageParcel::ClearAllErrorFlag();
    res = stub_->HandleIsFreeMultiWindow(data, reply);
    EXPECT_EQ(res, ERR_NONE);
}

/**
 * @tc.name: HandleRecoverWindowPropertyChangeFlag01
 * @tc.desc: test HandleRecoverWindowPropertyChangeFlag
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest2, HandleRecoverWindowPropertyChangeFlag01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;

    auto res = stub_->HandleRecoverWindowPropertyChangeFlag(data, reply);
    EXPECT_EQ(res, ERR_TRANSACTION_FAILED);

    data.WriteUint32(0);
    res = stub_->HandleRecoverWindowPropertyChangeFlag(data, reply);
    EXPECT_EQ(res, ERR_TRANSACTION_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    res = stub_->HandleRecoverWindowPropertyChangeFlag(data, reply);
    EXPECT_EQ(res, ERR_TRANSACTION_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: HandleSetWatermarkImageForApp01
 * @tc.desc: test HandleSetWatermarkImageForApp
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest2, HandleSetWatermarkImageForApp01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_SET_APP_WATERMARK_IMAGE);
    auto res = stub_->ProcessRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    res = stub_->HandleSetWatermarkImageForApp(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    res = stub_->HandleSetWatermarkImageForApp(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: HandleRecoverWatermarkImageForApp01
 * @tc.desc: test HandleRecoverWatermarkImageForApp
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest2, HandleRecoverWatermarkImageForApp01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_RECOVER_APP_WATERMARK_IMAGE);
    auto res = stub_->ProcessRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_NONE);

    data.WriteString("watermark");
    res = stub_->HandleRecoverWatermarkImageForApp(data, reply);
    EXPECT_EQ(res, ERR_NONE);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    data.WriteString("watermark");
    res = stub_->HandleRecoverWatermarkImageForApp(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: HandleUpdateSessionOcclusionStateListener
 * @tc.desc: test HandleUpdateSessionOcclusionStateListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest2, HandleUpdateSessionOcclusionStateListener, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_UPDATE_SESSION_OCCLUSION_STATE_LISTENER);
    auto res = stub_->ProcessRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    res = stub_->HandleUpdateSessionOcclusionStateListener(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteInt32(1);
    res = stub_->HandleUpdateSessionOcclusionStateListener(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    data.WriteBool(false);
    res = stub_->HandleUpdateSessionOcclusionStateListener(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: HandleGetRootUIContentRemoteObj01
 * @tc.desc: test HandleGetRootUIContentRemoteObj
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest2, HandleGetRootUIContentRemoteObj01, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    ASSERT_NE(stub_, nullptr);
    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_GET_ROOT_UI_CONTENT_REMOTE_OBJ);
    auto res = stub_->ProcessRemoteRequest(code, data, reply, option);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    DisplayId displayId = 1000;
    sptr<IRemoteObject> uiContentRemoteObj = nullptr;
    data.ReadUint64(displayId);
    res = stub_->HandleGetRootUIContentRemoteObj(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);

    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetReadUint64ErrorFlag(true);
    res = stub_->HandleGetRootUIContentRemoteObj(data, reply);
    EXPECT_EQ(res, ERR_INVALID_DATA);
    MockMessageParcel::SetReadUint64ErrorFlag(false);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: HandleCreateUIEffectController
 * @tc.desc: test HandleCreateUIEffectController
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest2, HandleCreateUIEffectController, Function | SmallTest | Level2)
{
    MockMessageParcel::ClearAllErrorFlag();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_CREATE_UI_EFFECT_CONTROLLER);
    sptr<IUIEffectControllerClient> client = sptr<UIEffectControllerClient>::MakeSptr();
    stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(stub_->HandleCreateUIEffectController(data, reply), ERR_INVALID_DATA);
    data.WriteRemoteObject(client->AsObject());
    stub_->HandleCreateUIEffectController(data, reply);
    sptr<MockSceneSessionManagerStub> stubMock = sptr<MockSceneSessionManagerStub>::MakeSptr();
    EXPECT_CALL(*stubMock, CreateUIEffectController(_, _, _)).WillOnce(Return(static_cast<WMError>(-1)));
    data.RewindRead(0);
    EXPECT_EQ(stubMock->HandleCreateUIEffectController(data, reply), ERR_INVALID_DATA);
    EXPECT_CALL(*stubMock, CreateUIEffectController(_, _, _)).WillOnce(DoAll(Return(WMError::WM_ERROR_NOT_SYSTEM_APP)));
    data.RewindRead(0);
    EXPECT_EQ(stubMock->HandleCreateUIEffectController(data, reply), ERR_NONE);
    EXPECT_CALL(*stubMock, CreateUIEffectController(_, _, _)).WillOnce(DoAll(SetArgReferee<2>(-1),
        Return(WMError::WM_OK)));
    data.RewindRead(0);
    EXPECT_EQ(stubMock->HandleCreateUIEffectController(data, reply), ERR_INVALID_DATA);
    EXPECT_CALL(*stubMock, CreateUIEffectController(_, _, _)).WillOnce(DoAll(SetArgReferee<1>(nullptr),
        SetArgReferee<2>(0), Return(WMError::WM_OK)));
    data.RewindRead(0);
    EXPECT_EQ(stubMock->HandleCreateUIEffectController(data, reply), ERR_INVALID_DATA);
    sptr<UIEffectController> server = sptr<UIEffectController>::MakeSptr(0, nullptr, nullptr);
    EXPECT_CALL(*stubMock, CreateUIEffectController(_, _, _)).WillOnce(DoAll(SetArgReferee<1>(server),
        SetArgReferee<2>(0), Return(WMError::WM_OK)));
    data.RewindRead(0);
    EXPECT_EQ(stubMock->HandleCreateUIEffectController(data, reply), ERR_NONE);
}

/**
 * @tc.name: HandleUpdateOutline
 * @tc.desc: test HandleUpdateOutline
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest2, HandleUpdateOutline, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = static_cast<uint32_t>(
        ISceneSessionManager::SceneSessionManagerMessage::TRANS_ID_UPDATE_OUTLINE);
    
    auto ret = stub_->ProcessRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, ERR_INVALID_DATA);

    sptr<IRemoteObject> remoteObject = sptr<MockIRemoteObject>::MakeSptr();
    data.WriteRemoteObject(remoteObject);
    ret = stub_->HandleUpdateOutline(data, reply);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleUpdateOutline01
 * @tc.desc: test HandleUpdateOutline
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest2, HandleUpdateOutline01, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    MessageParcel data;
    MessageParcel reply;

    sptr<IRemoteObject> remoteObject = sptr<MockIRemoteObject>::MakeSptr();
    data.WriteRemoteObject(remoteObject);
    OutlineParams params;
    data.WriteParcelable(&params);
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    auto ret = stub_->HandleUpdateOutline(data, reply);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
}

/**
 * @tc.name: HandleUpdateOutline02
 * @tc.desc: test HandleUpdateOutline
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerStubTest2, HandleUpdateOutline02, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    MessageParcel data;
    MessageParcel reply;

    sptr<IRemoteObject> remoteObject = sptr<MockIRemoteObject>::MakeSptr();
    data.WriteRemoteObject(remoteObject);
    OutlineParams params;
    data.WriteParcelable(&params);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);
    auto ret = stub_->HandleUpdateOutline(data, reply);
    EXPECT_EQ(ret, ERR_INVALID_DATA);
}
} // namespace
} // namespace Rosen
} // namespace OHOS

