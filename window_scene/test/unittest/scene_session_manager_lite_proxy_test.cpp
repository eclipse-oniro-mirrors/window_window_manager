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
 
#include "iremote_object_mocker.h"
#include "ws_common.h"
#include "mock_message_parcel.h"
#include "mock/mock_session.h"
#include "mock/mock_session_stage.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_manager/include/zidl/scene_session_manager_lite_interface.h"
#include "session_manager/include/zidl/scene_session_manager_lite_proxy.h"
#include "session_manager/include/zidl/session_router_stack_listener_stub.h"
#include "session_manager/include/zidl/pip_change_listener_stub.h"
#include "window_manager_agent.h"
 
using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class sceneSessionManagerLiteProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<IRemoteObject> iRemoteObjectMocker;

private:
    sptr<MockIRemoteObject> iRemoteObjectMocker_ {nullptr};
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy_ {nullptr};
};
 
void sceneSessionManagerLiteProxyTest::SetUpTestCase()
{
}
 
void sceneSessionManagerLiteProxyTest::TearDownTestCase()
{
}
 
void sceneSessionManagerLiteProxyTest::SetUp()
{
    iRemoteObjectMocker_ = sptr<MockIRemoteObject>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker_, nullptr);
    sceneSessionManagerLiteProxy_ = sptr<SceneSessionManagerLiteProxy>::MakeSptr(iRemoteObjectMocker_);
    ASSERT_NE(sceneSessionManagerLiteProxy_, nullptr);
}
 
void sceneSessionManagerLiteProxyTest::TearDown()
{
    iRemoteObjectMocker_ = nullptr;
    sceneSessionManagerLiteProxy_ = nullptr;
    MockMessageParcel::ClearAllErrorFlag();
}
 
namespace {
/**
 * @tc.name: PendingSessionToBackground
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, PendingSessionToBackground, TestSize.Level1)
{
    BackgroundParams params;
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(token);
    ASSERT_NE(sceneSessionManagerLiteProxy, nullptr);
    MockMessageParcel::ClearAllErrorFlag();
    WSError errCode = sceneSessionManagerLiteProxy->PendingSessionToBackground(token, params);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->PendingSessionToBackground(token, params);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    MockMessageParcel::SetWriteRemoteObjectErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->PendingSessionToBackground(token, params);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    MockMessageParcel::SetWriteRemoteObjectErrorFlag(false);
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->PendingSessionToBackground(token, params);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    MockMessageParcel::SetWriteRemoteObjectErrorFlag(false);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->PendingSessionToBackground(token, params);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    MockMessageParcel::SetWriteRemoteObjectErrorFlag(false);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);
    MockMessageParcel::SetWriteBoolErrorFlag(false);
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->PendingSessionToBackground(token, params);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    MockMessageParcel::SetWriteRemoteObjectErrorFlag(false);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);
    MockMessageParcel::SetWriteBoolErrorFlag(false);
    MockMessageParcel::SetWriteParcelableErrorFlag(false);
    errCode = sceneSessionManagerLiteProxy->PendingSessionToBackground(token, params);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    MockMessageParcel::SetWriteRemoteObjectErrorFlag(false);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);
    MockMessageParcel::SetWriteBoolErrorFlag(false);
    MockMessageParcel::SetWriteParcelableErrorFlag(false);
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->PendingSessionToBackground(token, params);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: UpdateAnimationSpeedWithPid
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, UpdateAnimationSpeedWithPid, TestSize.Level1)
{
    sptr<MockIRemoteObject> iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    ASSERT_NE(iRemoteObjectMocker, nullptr);
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sceneSessionManagerLiteProxy, nullptr);

    MockMessageParcel::ClearAllErrorFlag();
    EXPECT_EQ(WMError::WM_OK, sceneSessionManagerLiteProxy->UpdateAnimationSpeedWithPid(10000, 2.0f));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    WMError errCode = sceneSessionManagerLiteProxy->UpdateAnimationSpeedWithPid(10000, 2.0f);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->UpdateAnimationSpeedWithPid(10000, 2.0f);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);

    MockMessageParcel::SetWriteFloatErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->UpdateAnimationSpeedWithPid(10000, 2.0f);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteFloatErrorFlag(false);

    sptr<SceneSessionManagerLiteProxy> nullptrProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    errCode = nullptrProxy->UpdateAnimationSpeedWithPid(10000, 2.0f);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);

    iRemoteObjectMocker->SetRequestResult(1);
    errCode = sceneSessionManagerLiteProxy->UpdateAnimationSpeedWithPid(10000, 2.0f);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    iRemoteObjectMocker->SetRequestResult(0);

    MockMessageParcel::SetReadInt32ErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->UpdateAnimationSpeedWithPid(10000, 2.0f);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: SetGlobalDragResizeType
 * @tc.desc: SetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, SetGlobalDragResizeType, TestSize.Level1)
{
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    // remote nullptr
    sptr<SceneSessionManagerLiteProxy> nullptrProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    auto errCode = nullptrProxy->SetGlobalDragResizeType(dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_TRUE(sceneSessionManagerLiteProxy != nullptr);
    // write token failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->SetGlobalDragResizeType(dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    // write value failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->SetGlobalDragResizeType(dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    // send failed
    MockMessageParcel::ClearAllErrorFlag();
    iRemoteObjectMocker->SetRequestResult(1);
    errCode = sceneSessionManagerLiteProxy->SetGlobalDragResizeType(dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    iRemoteObjectMocker->SetRequestResult(0);
    // read failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetReadInt32ErrorFlag(1);
    errCode = sceneSessionManagerLiteProxy->SetGlobalDragResizeType(dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    // pass
    MockMessageParcel::ClearAllErrorFlag();
    errCode = sceneSessionManagerLiteProxy->SetGlobalDragResizeType(dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_OK);
}

/**
 * @tc.name: GetGlobalDragResizeType
 * @tc.desc: GetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, GetGlobalDragResizeType, TestSize.Level1)
{
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    // remote nullptr
    sptr<SceneSessionManagerLiteProxy> nullptrProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    auto errCode = nullptrProxy->GetGlobalDragResizeType(dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_TRUE(sceneSessionManagerLiteProxy != nullptr);
    // write token failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->GetGlobalDragResizeType(dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    // send failed
    MockMessageParcel::ClearAllErrorFlag();
    iRemoteObjectMocker->SetRequestResult(1);
    errCode = sceneSessionManagerLiteProxy->GetGlobalDragResizeType(dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    iRemoteObjectMocker->SetRequestResult(0);
    // read failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetReadInt32ErrorFlag(1);
    errCode = sceneSessionManagerLiteProxy->GetGlobalDragResizeType(dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: SetAppDragResizeType
 * @tc.desc: SetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, SetAppDragResizeType, TestSize.Level1)
{
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    const std::string bundleName = "test";
    // remote nullptr
    sptr<SceneSessionManagerLiteProxy> nullptrProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    auto errCode = nullptrProxy->SetAppDragResizeType(bundleName, dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_TRUE(sceneSessionManagerLiteProxy != nullptr);
    // write token failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->SetAppDragResizeType(bundleName, dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    // write value failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteStringErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->SetAppDragResizeType(bundleName, dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->SetAppDragResizeType(bundleName, dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    // send failed
    MockMessageParcel::ClearAllErrorFlag();
    iRemoteObjectMocker->SetRequestResult(1);
    errCode = sceneSessionManagerLiteProxy->SetAppDragResizeType(bundleName, dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    iRemoteObjectMocker->SetRequestResult(0);
    // read failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetReadInt32ErrorFlag(1);
    errCode = sceneSessionManagerLiteProxy->SetAppDragResizeType(bundleName, dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    // pass
    MockMessageParcel::ClearAllErrorFlag();
    errCode = sceneSessionManagerLiteProxy->SetAppDragResizeType(bundleName, dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_OK);
}

/**
 * @tc.name: GetAppDragResizeType
 * @tc.desc: GetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, GetAppDragResizeType, TestSize.Level1)
{
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    const std::string bundleName = "test";
    // remote nullptr
    sptr<SceneSessionManagerLiteProxy> nullptrProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    auto errCode = nullptrProxy->GetAppDragResizeType(bundleName, dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_TRUE(sceneSessionManagerLiteProxy != nullptr);
    // write token failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->GetAppDragResizeType(bundleName, dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    // write failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteStringErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->GetAppDragResizeType(bundleName, dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    // send failed
    MockMessageParcel::ClearAllErrorFlag();
    iRemoteObjectMocker->SetRequestResult(1);
    errCode = sceneSessionManagerLiteProxy->GetAppDragResizeType(bundleName, dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    iRemoteObjectMocker->SetRequestResult(0);
    // read failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetReadInt32ErrorFlag(1);
    errCode = sceneSessionManagerLiteProxy->GetAppDragResizeType(bundleName, dragResizeType);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: SetAppKeyFramePolicy
 * @tc.desc: SetAppKeyFramePolicy
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, SetAppKeyFramePolicy, TestSize.Level1)
{
    const std::string bundleName = "test";
    KeyFramePolicy keyFramePolicy;
    keyFramePolicy.dragResizeType_ = DragResizeType::RESIZE_EACH_FRAME;
    // remote nullptr
    sptr<SceneSessionManagerLiteProxy> nullptrProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    auto errCode = nullptrProxy->SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_TRUE(sceneSessionManagerLiteProxy != nullptr);
    // write token failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    // write value failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteStringErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    // send failed
    MockMessageParcel::ClearAllErrorFlag();
    iRemoteObjectMocker->SetRequestResult(1);
    errCode = sceneSessionManagerLiteProxy->SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    iRemoteObjectMocker->SetRequestResult(0);
    // read failed
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetReadInt32ErrorFlag(1);
    errCode = sceneSessionManagerLiteProxy->SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    // pass
    MockMessageParcel::ClearAllErrorFlag();
    errCode = sceneSessionManagerLiteProxy->SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    EXPECT_EQ(errCode, WMError::WM_OK);
}

/**
 * @tc.name: PendingSessionToBackgroundByPersistentId
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, PendingSessionToBackgroundByPersistentId, TestSize.Level1)
{
    sptr<MockIRemoteObject> iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sceneSessionManagerLiteProxy, nullptr);
    const int32_t persistentId = 1;
    bool shouldBackToCaller = true;
    MockMessageParcel::ClearAllErrorFlag();
    WSError errCode =
        sceneSessionManagerLiteProxy->PendingSessionToBackgroundByPersistentId(persistentId, shouldBackToCaller);
    EXPECT_EQ(errCode, WSError::WS_OK);
 
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->PendingSessionToBackgroundByPersistentId(persistentId, shouldBackToCaller);
    EXPECT_EQ(errCode, WSError::WS_ERROR_INVALID_PARAM);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
 
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->PendingSessionToBackgroundByPersistentId(persistentId, shouldBackToCaller);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);
 
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->PendingSessionToBackgroundByPersistentId(persistentId, shouldBackToCaller);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteBoolErrorFlag(false);
 
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy2 =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    errCode = sceneSessionManagerLiteProxy2->PendingSessionToBackgroundByPersistentId(persistentId, shouldBackToCaller);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
 
    iRemoteObjectMocker->SetRequestResult(1);
    errCode = sceneSessionManagerLiteProxy->PendingSessionToBackgroundByPersistentId(persistentId, shouldBackToCaller);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    iRemoteObjectMocker->SetRequestResult(0);
 
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->PendingSessionToBackgroundByPersistentId(persistentId, shouldBackToCaller);
    MockMessageParcel::SetReadInt32ErrorFlag(false);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
}

/**
 * @tc.name: UpdateWindowModeByIdForUITest01
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, UpdateWindowModeByIdForUITest01, TestSize.Level1)
{
    sptr<MockIRemoteObject> iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(iRemoteObjectMocker);
    const int32_t windowId = 1;
    const int32_t updateMode = 1;
    MockMessageParcel::ClearAllErrorFlag();
    WMError errCode =
        sceneSessionManagerLiteProxy->UpdateWindowModeByIdForUITest(windowId, updateMode);
    EXPECT_EQ(errCode, WMError::WM_OK);
 
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->UpdateWindowModeByIdForUITest(windowId, updateMode);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
 
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->UpdateWindowModeByIdForUITest(windowId, updateMode);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);
 
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy2 =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    errCode = sceneSessionManagerLiteProxy2->UpdateWindowModeByIdForUITest(windowId, updateMode);
    EXPECT_EQ(errCode, WMError::WM_ERROR_NULLPTR);
 
    iRemoteObjectMocker->SetRequestResult(1);
    errCode = sceneSessionManagerLiteProxy->UpdateWindowModeByIdForUITest(windowId, updateMode);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    iRemoteObjectMocker->SetRequestResult(0);
 
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->UpdateWindowModeByIdForUITest(windowId, updateMode);
    MockMessageParcel::SetReadInt32ErrorFlag(false);
    EXPECT_EQ(errCode, WMError::WM_OK);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: UpdateScreenLockStatusForApp
 * @tc.desc: update screen lock status
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, UpdateScreenLockStatusForApp, TestSize.Level1)
{
    auto tempProxy = sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    auto ret = tempProxy->UpdateScreenLockStatusForApp("", true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::ClearAllErrorFlag();
    EXPECT_EQ(proxy->UpdateScreenLockStatusForApp("", true), WMError::WM_OK);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = proxy->UpdateScreenLockStatusForApp("a", true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteStringErrorFlag(true);
    ret = proxy->UpdateScreenLockStatusForApp("b", false);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteStringErrorFlag(false);

    MockMessageParcel::SetWriteBoolErrorFlag(true);
    ret = proxy->UpdateScreenLockStatusForApp("c", false);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteBoolErrorFlag(false);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->UpdateScreenLockStatusForApp("d", true);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: SendPointerEventForHover
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, SendPointerEventForHover, TestSize.Level1)
{
    sptr<MockIRemoteObject> iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sceneSessionManagerLiteProxy, nullptr);
    const int32_t persistentId = 1;
    bool shouldBackToCaller = true;
    MockMessageParcel::ClearAllErrorFlag();
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    WSError errCode = sceneSessionManagerLiteProxy->SendPointerEventForHover(pointerEvent);
    EXPECT_EQ(errCode, WSError::WS_OK);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->SendPointerEventForHover(pointerEvent);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    iRemoteObjectMocker->SetRequestResult(1);
    errCode = sceneSessionManagerLiteProxy->SendPointerEventForHover(pointerEvent);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    iRemoteObjectMocker->SetRequestResult(0);

    MockMessageParcel::SetReadInt32ErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->SendPointerEventForHover(pointerEvent);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadInt32ErrorFlag(false);
}

/**
 * @tc.name: RegisterWindowPropertyChangeAgent
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, RegisterWindowPropertyChangeAgent, TestSize.Level1)
{
    WindowInfoKey windowInfoKey = WindowInfoKey::DISPLAY_ID;
    uint32_t interestInfo = 0;
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();

    auto tempProxy = sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    auto ret = tempProxy->RegisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::ClearAllErrorFlag();
    ret = proxy->RegisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_OK);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = proxy->RegisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    ret = proxy->RegisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    ret = proxy->RegisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteUint32ErrorFlag(false);

    ret = proxy->RegisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, nullptr);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->RegisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: UnregisterWindowPropertyChangeAgent
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, UnregisterWindowPropertyChangeAgent, TestSize.Level1)
{
    WindowInfoKey windowInfoKey = WindowInfoKey::DISPLAY_ID;
    uint32_t interestInfo = 0;
    sptr<IWindowManagerAgent> windowManagerAgent = sptr<WindowManagerAgent>::MakeSptr();

    auto tempProxy = sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    auto ret = tempProxy->UnregisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::ClearAllErrorFlag();
    ret = proxy->UnregisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_OK);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = proxy->UnregisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    ret = proxy->UnregisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    ret = proxy->UnregisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteUint32ErrorFlag(false);

    ret = proxy->UnregisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, nullptr);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->UnregisterWindowPropertyChangeAgent(windowInfoKey, interestInfo, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: RecoverWindowPropertyChangeFlag
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, RecoverWindowPropertyChangeFlag, TestSize.Level1)
{
    uint32_t observedFlags = 0;
    uint32_t interestedFlags = 0;

    auto tempProxy = sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    auto ret = tempProxy->RecoverWindowPropertyChangeFlag(observedFlags, interestedFlags);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);

    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<SceneSessionManagerLiteProxy>::MakeSptr(remoteMocker);
    ASSERT_NE(proxy, nullptr);

    MockMessageParcel::ClearAllErrorFlag();
    ret = proxy->RecoverWindowPropertyChangeFlag(observedFlags, interestedFlags);
    EXPECT_EQ(ret, WMError::WM_OK);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ret = proxy->RecoverWindowPropertyChangeFlag(observedFlags, interestedFlags);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    ret = proxy->RecoverWindowPropertyChangeFlag(observedFlags, interestedFlags);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteUint32ErrorFlag(false);

    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ret = proxy->RecoverWindowPropertyChangeFlag(observedFlags, interestedFlags);
    EXPECT_EQ(ret, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: IsFocusWindowParent
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, IsFocusWindowParent, TestSize.Level1)
{
    sptr<MockIRemoteObject> iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sceneSessionManagerLiteProxy, nullptr);
    MockMessageParcel::ClearAllErrorFlag();
    sptr<IRemoteObject> token = nullptr;
    bool isParent = false;
    WSError errCode = sceneSessionManagerLiteProxy->IsFocusWindowParent(token, isParent);
    EXPECT_EQ(errCode, WSError::WS_ERROR_INVALID_PARAM);

    token = sptr<MockIRemoteObject>::MakeSptr();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->IsFocusWindowParent(token, isParent);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    iRemoteObjectMocker->SetRequestResult(1);
    errCode = sceneSessionManagerLiteProxy->IsFocusWindowParent(token, isParent);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    iRemoteObjectMocker->SetRequestResult(0);

    MockMessageParcel::SetReadBoolErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->IsFocusWindowParent(token, isParent);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadBoolErrorFlag(false);

    MockMessageParcel::SetReadInt32ErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->IsFocusWindowParent(token, isParent);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetReadInt32ErrorFlag(false);
}

/**
 * @tc.name: NotifyAppUseControlList
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, NotifyAppUseControlList, TestSize.Level1)
{
    sptr<MockIRemoteObject> iRemoteObjectMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(iRemoteObjectMocker);
    ASSERT_NE(sceneSessionManagerLiteProxy, nullptr);
    MockMessageParcel::ClearAllErrorFlag();
    ControlAppType type = ControlAppType::DLP;
    int32_t userId = 100;
    std::vector<AppUseControlInfo> controlList;
    AppUseControlInfo controlInfo;
    controlList.push_back(controlInfo);
    WMError errCode = sceneSessionManagerLiteProxy->NotifyAppUseControlList(type, userId, controlList);
    EXPECT_EQ(errCode, WSError::WS_OK);
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    errCode = sceneSessionManagerLiteProxy->NotifyAppUseControlList(type, userId, controlList);
    EXPECT_EQ(errCode, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: PendingSessionToForeground_Fail
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, PendingSessionToForeground_Fail, TestSize.Level1)
{
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    int32_t windowMode = static_cast<int32_t>(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(token);
    ASSERT_NE(sceneSessionManagerLiteProxy, nullptr);
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    WSError errCode = sceneSessionManagerLiteProxy->PendingSessionToForeground(token, windowMode);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: PendingSessionToForeground_Success
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(sceneSessionManagerLiteProxyTest, PendingSessionToForeground_Success, TestSize.Level1)
{
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    int32_t windowMode = static_cast<int32_t>(WindowMode::WINDOW_MODE_FULLSCREEN);
    sptr<SceneSessionManagerLiteProxy> sceneSessionManagerLiteProxy =
        sptr<SceneSessionManagerLiteProxy>::MakeSptr(token);
    ASSERT_NE(sceneSessionManagerLiteProxy, nullptr);
    MockMessageParcel::ClearAllErrorFlag();
    WSError errCode = sceneSessionManagerLiteProxy->PendingSessionToForeground(token, windowMode);
    EXPECT_EQ(errCode, WSError::WS_OK);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, UnregPipChgListenerByScreenId_WriteTokenFailed, TestSize.Level1)
{
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(sceneSessionManagerLiteProxy_->UnregisterPipChgListenerByScreenId(1), WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, UnregPipChgListenerByScreenId_WriteIntFailed, TestSize.Level1)
{
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    EXPECT_EQ(sceneSessionManagerLiteProxy_->UnregisterPipChgListenerByScreenId(1), WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, UnregPipChgListenerByScreenId_ReadIntFailed, TestSize.Level1)
{
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    EXPECT_EQ(sceneSessionManagerLiteProxy_->UnregisterPipChgListenerByScreenId(1), WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, UnregPipChgListenerByScreenId_SendReqFailed, TestSize.Level1)
{
    iRemoteObjectMocker_->SetRequestResult(1);
    EXPECT_EQ(sceneSessionManagerLiteProxy_->UnregisterPipChgListenerByScreenId(1), WMError::WM_ERROR_IPC_FAILED);
    iRemoteObjectMocker_->SetRequestResult(0);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, UnregPipChgListenerByScreenId_RemoteNullFailed, TestSize.Level1)
{
    sptr<SceneSessionManagerLiteProxy> liteProxyNullRemote = sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    ASSERT_NE(liteProxyNullRemote, nullptr);
    EXPECT_EQ(liteProxyNullRemote->UnregisterPipChgListenerByScreenId(1), WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, RegisterPipChgListenerByScreenId_ShouldReturnOk_WhenNotReg, TestSize.Level1)
{
    EXPECT_EQ(sceneSessionManagerLiteProxy_->UnregisterPipChgListenerByScreenId(1), WMError::WM_OK);
}

class MockPipChgListener : public PipChangeListenerStub {
public:
    void OnPipStart(int32_t windowId) override {};
};

HWTEST_F(sceneSessionManagerLiteProxyTest, RegPipChgListenerByScreenId_Success, TestSize.Level1)
{
    sptr<IPipChangeListener> listener = sptr<MockPipChgListener>::MakeSptr();
    EXPECT_EQ(sceneSessionManagerLiteProxy_->RegisterPipChgListenerByScreenId(1, listener), WMError::WM_OK);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, UnregPipChgListenerByScreenId_ShouldReturnOk_WhenRegOK, TestSize.Level1)
{
    sptr<IPipChangeListener> listener = sptr<MockPipChgListener>::MakeSptr();
    EXPECT_EQ(sceneSessionManagerLiteProxy_->RegisterPipChgListenerByScreenId(1, listener), WMError::WM_OK);
    EXPECT_EQ(sceneSessionManagerLiteProxy_->UnregisterPipChgListenerByScreenId(1), WMError::WM_OK);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, RegPipChgListenerByScreenId_WriteTokenFailed, TestSize.Level1)
{
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    auto result = sceneSessionManagerLiteProxy_->RegisterPipChgListenerByScreenId(1, nullptr);
    EXPECT_EQ(result, WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, RegPipChgListenerByScreenId_WriteIntFailed, TestSize.Level1)
{
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    auto result = sceneSessionManagerLiteProxy_->RegisterPipChgListenerByScreenId(1, nullptr);
    EXPECT_EQ(result, WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, RegPipChgListenerByScreenId_NullListenerFailed, TestSize.Level1)
{
    auto result = sceneSessionManagerLiteProxy_->RegisterPipChgListenerByScreenId(1, nullptr);
    EXPECT_EQ(result, WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, RegPipChgListenerByScreenId_WriteRemoteObjFailed, TestSize.Level1)
{
    MockMessageParcel::SetWriteRemoteObjectErrorFlag(true);
    sptr<IPipChangeListener> listener = sptr<MockPipChgListener>::MakeSptr();
    auto result = sceneSessionManagerLiteProxy_->RegisterPipChgListenerByScreenId(1, listener);
    EXPECT_EQ(result, WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, RegPipChgListenerByScreenId_SendReqFailed, TestSize.Level1)
{
    iRemoteObjectMocker_->SetRequestResult(1);
    sptr<IPipChangeListener> listener = sptr<MockPipChgListener>::MakeSptr();
    auto result = sceneSessionManagerLiteProxy_->RegisterPipChgListenerByScreenId(1, listener);
    EXPECT_EQ(result, WMError::WM_ERROR_IPC_FAILED);
    iRemoteObjectMocker_->SetRequestResult(0);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, RegPipChgListenerByScreenId_RemoteNullFailed, TestSize.Level1)
{
    sptr<SceneSessionManagerLiteProxy> liteProxyNullRemote = sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    ASSERT_NE(liteProxyNullRemote, nullptr);
    sptr<IPipChangeListener> listener = sptr<MockPipChgListener>::MakeSptr();
    EXPECT_EQ(liteProxyNullRemote->RegisterPipChgListenerByScreenId(1, listener), WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, RegPipChgListenerByScreenId_ReadIntFailed, TestSize.Level1)
{
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    sptr<IPipChangeListener> listener = sptr<MockPipChgListener>::MakeSptr();
    auto result = sceneSessionManagerLiteProxy_->RegisterPipChgListenerByScreenId(1, listener);
    EXPECT_EQ(result, WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, SetPipEnableByScreenId_Success, TestSize.Level1)
{
    EXPECT_EQ(sceneSessionManagerLiteProxy_->SetPipEnableByScreenId(1, false), WMError::WM_OK);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, SetPipEnableByScreenId_WriteTokenFailed, TestSize.Level1)
{
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(sceneSessionManagerLiteProxy_->SetPipEnableByScreenId(1, false), WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, SetPipEnableByScreenId_WriteIntFailed, TestSize.Level1)
{
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    EXPECT_EQ(sceneSessionManagerLiteProxy_->SetPipEnableByScreenId(1, false), WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, SetPipEnableByScreenId_WriteBoolFailed, TestSize.Level1)
{
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    EXPECT_EQ(sceneSessionManagerLiteProxy_->SetPipEnableByScreenId(1, false), WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, SetPipEnableByScreenId_SendReqFailed, TestSize.Level1)
{
    iRemoteObjectMocker_->SetRequestResult(1);
    EXPECT_EQ(sceneSessionManagerLiteProxy_->SetPipEnableByScreenId(1, false), WMError::WM_ERROR_IPC_FAILED);
    iRemoteObjectMocker_->SetRequestResult(0);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, SetPipEnableByScreenId_RemoteNullFailed, TestSize.Level1)
{
    sptr<SceneSessionManagerLiteProxy> liteProxyNullRemote = sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    ASSERT_NE(liteProxyNullRemote, nullptr);
    EXPECT_EQ(liteProxyNullRemote->SetPipEnableByScreenId(1, false), WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, SetPipEnableByScreenId_ReadIntFailed, TestSize.Level1)
{
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    EXPECT_EQ(sceneSessionManagerLiteProxy_->SetPipEnableByScreenId(1, false), WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, UnsetPipEnableByScreenId_Success, TestSize.Level1)
{
    EXPECT_EQ(sceneSessionManagerLiteProxy_->UnsetPipEnableByScreenId(1), WMError::WM_OK);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, UnsetPipEnableByScreenId_WriteTokenFailed, TestSize.Level1)
{
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(sceneSessionManagerLiteProxy_->UnsetPipEnableByScreenId(1), WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, UnsetPipEnableByScreenId_WriteIntFailed, TestSize.Level1)
{
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    EXPECT_EQ(sceneSessionManagerLiteProxy_->UnsetPipEnableByScreenId(1), WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, UnsetPipEnableByScreenId_SendReqFailed, TestSize.Level1)
{
    iRemoteObjectMocker_->SetRequestResult(1);
    EXPECT_EQ(sceneSessionManagerLiteProxy_->UnsetPipEnableByScreenId(1), WMError::WM_ERROR_IPC_FAILED);
    iRemoteObjectMocker_->SetRequestResult(0);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, UnsetPipEnableByScreenId_RemoteNullFailed, TestSize.Level1)
{
    sptr<SceneSessionManagerLiteProxy> liteProxyNullRemote = sptr<SceneSessionManagerLiteProxy>::MakeSptr(nullptr);
    ASSERT_NE(liteProxyNullRemote, nullptr);
    EXPECT_EQ(liteProxyNullRemote->UnsetPipEnableByScreenId(1), WMError::WM_ERROR_IPC_FAILED);
}

HWTEST_F(sceneSessionManagerLiteProxyTest, UnsetPipEnableByScreenId_ReadIntFailed, TestSize.Level1)
{
    MockMessageParcel::SetReadInt32ErrorFlag(true);
    EXPECT_EQ(sceneSessionManagerLiteProxy_->UnsetPipEnableByScreenId(1), WMError::WM_ERROR_IPC_FAILED);
}
}
}
}