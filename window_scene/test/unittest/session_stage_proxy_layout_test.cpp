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

#include "iremote_object_mocker.h"
#include "mock_message_parcel.h"
#include "session/container/include/zidl/session_stage_proxy.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type,
                       const LogLevel level,
                       const unsigned int domain,
                       const char* tag,
                       const char* msg)
    {
        g_logMsg += msg;
    }
}
class SessionStageProxyLayoutTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<SessionStageProxy> sessionStage_ = sptr<SessionStageProxy>::MakeSptr(iRemoteObjectMocker);
};

void SessionStageProxyLayoutTest::SetUpTestCase() {}

void SessionStageProxyLayoutTest::TearDownTestCase() {}

void SessionStageProxyLayoutTest::SetUp() {}

void SessionStageProxyLayoutTest::TearDown() {}

namespace {
/**
 * @tc.name: NotifySingleHandTransformChange
 * @tc.desc: test function : NotifySingleHandTransformChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, NotifySingleHandTransformChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionStageProxyLayoutTest: NotifySingleHandTransformChange start";
    SingleHandTransform singleHandTransform;
    sessionStage_->NotifySingleHandTransformChange(singleHandTransform);
    ASSERT_TRUE((sessionStage_ != nullptr));
    GTEST_LOG_(INFO) << "SessionStageProxyLayoutTest: NotifySingleHandTransformChange end";
}

/**
 * @tc.name: NotifyLayoutFinishAfterWindowModeChange
 * @tc.desc: test function : NotifyLayoutFinishAfterWindowModeChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, NotifyLayoutFinishAfterWindowModeChange, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    GTEST_LOG_(INFO) << "SessionStageProxyLayoutTest: NotifyLayoutFinishAfterWindowModeChange start";
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    auto ret = sessionStage_->NotifyLayoutFinishAfterWindowModeChange(WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    ret = sessionStage_->NotifyLayoutFinishAfterWindowModeChange(WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteUint32ErrorFlag(false);

    sptr<SessionStageProxy> sessionStage = sptr<SessionStageProxy>::MakeSptr(nullptr);
    ret = sessionStage->NotifyLayoutFinishAfterWindowModeChange(WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);

    ret = sessionStage_->NotifyLayoutFinishAfterWindowModeChange(WindowMode::WINDOW_MODE_FULLSCREEN);
    EXPECT_EQ(WSError::WS_OK, ret);
    MockMessageParcel::ClearAllErrorFlag();
    GTEST_LOG_(INFO) << "SessionStageProxyLayoutTest: NotifyLayoutFinishAfterWindowModeChange end";
}

/**
 * @tc.name: TestNotifySubWindowAfterParentWindowSizeChange
 * @tc.desc: Test NotifySubWindowAfterParentWindowSizeChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, TestNotifySubWindowAfterParentWindowSizeChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionStageProxyLayoutTest: NotifySubWindowAfterParentWindowSizeChange start";
    Rect rect = { 10, 20, 100, 200 };

    // Case 1: Failed to write interface token
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage_->NotifySubWindowAfterParentWindowSizeChange(rect));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // Case 2: Failed to write rect
    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, sessionStage_->NotifySubWindowAfterParentWindowSizeChange(rect));
    MockMessageParcel::SetWriteInt32ErrorFlag(false);

    // Case 3: remote is nullptr
    sptr<SessionStageProxy> nullProxy = sptr<SessionStageProxy>::MakeSptr(nullptr);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, nullProxy->NotifySubWindowAfterParentWindowSizeChange(rect));

    // Case 5: Failed to send request
    auto remoteMock = sptr<MockIRemoteObject>::MakeSptr();
    remoteMock->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionStageProxy> failSendProxy = sptr<SessionStageProxy>::MakeSptr(remoteMock);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, failSendProxy->NotifySubWindowAfterParentWindowSizeChange(rect));

    // Case 6: Success
    remoteMock->sendRequestResult_ = ERR_NONE;
    sptr<SessionStageProxy> successProxy = sptr<SessionStageProxy>::MakeSptr(remoteMock);
    EXPECT_EQ(WSError::WS_OK, successProxy->NotifySubWindowAfterParentWindowSizeChange(rect));
    GTEST_LOG_(INFO) << "SessionStageProxyLayoutTest: NotifySubWindowAfterParentWindowSizeChange end";
}

/**
 * @tc.name: NotifySubWindowAfterParentWindowStatusChange
 * @tc.desc: test function : NotifySubWindowAfterParentWindowStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, NotifySubWindowAfterParentWindowStatusChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionStageProxyLayoutTest: NotifySubWindowAfterParentWindowStatusChange start";
    ASSERT_TRUE((sessionStage_ != nullptr));
    MockMessageParcel::ClearAllErrorFlag();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    auto ret = sessionStage_->NotifySubWindowAfterParentWindowStatusChange(WindowMode::WINDOW_MODE_FULLSCREEN,
        MaximizeMode::MODE_AVOID_SYSTEM_BAR, true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    ret = sessionStage_->NotifySubWindowAfterParentWindowStatusChange(WindowMode::WINDOW_MODE_FULLSCREEN,
        MaximizeMode::MODE_AVOID_SYSTEM_BAR, true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);

    MockMessageParcel::SetWriteUint32ErrorFlag(true);
    ret = sessionStage_->NotifySubWindowAfterParentWindowStatusChange(WindowMode::WINDOW_MODE_FULLSCREEN,
        MaximizeMode::MODE_AVOID_SYSTEM_BAR, true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteUint32ErrorFlag(false);

    MockMessageParcel::SetWriteBoolErrorFlag(true);
    ret = sessionStage_->NotifySubWindowAfterParentWindowStatusChange(WindowMode::WINDOW_MODE_FULLSCREEN,
        MaximizeMode::MODE_AVOID_SYSTEM_BAR, true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, ret);
    MockMessageParcel::SetWriteBoolErrorFlag(false);

    // Case 3: remote is nullptr
    sptr<SessionStageProxy> nullProxy = sptr<SessionStageProxy>::MakeSptr(nullptr);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED,
        nullProxy->NotifySubWindowAfterParentWindowStatusChange(WindowMode::WINDOW_MODE_FULLSCREEN,
            MaximizeMode::MODE_AVOID_SYSTEM_BAR, true));

    auto remoteMock = sptr<MockIRemoteObject>::MakeSptr();
    remoteMock->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionStageProxy> failSendProxy = sptr<SessionStageProxy>::MakeSptr(remoteMock);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED,
        failSendProxy->NotifySubWindowAfterParentWindowStatusChange(WindowMode::WINDOW_MODE_FULLSCREEN,
        MaximizeMode::MODE_AVOID_SYSTEM_BAR, true));

    remoteMock->sendRequestResult_ = ERROR_NONE;
    sptr<SessionStageProxy> successProxy = sptr<SessionStageProxy>::MakeSptr(remoteMock);
    EXPECT_EQ(WSError::WS_OK,
        successProxy->NotifySubWindowAfterParentWindowStatusChange(WindowMode::WINDOW_MODE_FULLSCREEN,
        MaximizeMode::MODE_AVOID_SYSTEM_BAR, true));

    MockMessageParcel::ClearAllErrorFlag();
    GTEST_LOG_(INFO) << "SessionStageProxyLayoutTest: NotifySubWindowAfterParentWindowStatusChange end";
}

/**
 * @tc.name: UpdateWindowModeForUITest01
 * @tc.desc: test function : UpdateWindowModeForUITest
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, UpdateWindowModeForUITest01, TestSize.Level1)
{
    TLOGI(WmsLogTag::WMS_LAYOUT, "SessionStageProxyLayoutTest: UpdateWindowModeForUITest01 start");
    GTEST_LOG_(INFO) << "SessionStageProxyLayoutTest: UpdateWindowModeForUITest01 start";
    const int32_t updateMode = 1;
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SessionStageProxy> sessionStageProxy = sptr<SessionStageProxy>::MakeSptr(remoteMocker);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    WMError errCode = sessionStageProxy->UpdateWindowModeForUITest(updateMode);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    errCode = sessionStageProxy->UpdateWindowModeForUITest(updateMode);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInt32ErrorFlag(false);

    remoteMocker->SetRequestResult(1);
    errCode = sessionStageProxy->UpdateWindowModeForUITest(updateMode);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(0);

    MockMessageParcel::SetReadInt32ErrorFlag(true);
    errCode = sessionStageProxy->UpdateWindowModeForUITest(updateMode);
    MockMessageParcel::SetReadInt32ErrorFlag(false);
    EXPECT_EQ(errCode, WMError::WM_OK);

    sptr<SessionStageProxy> sessionStageProxy2 = sptr<SessionStageProxy>::MakeSptr(nullptr);
    errCode = sessionStageProxy2->UpdateWindowModeForUITest(updateMode);
    EXPECT_EQ(errCode, WMError::WM_ERROR_IPC_FAILED);

    MockMessageParcel::ClearAllErrorFlag();
    TLOGI(WmsLogTag::WMS_LAYOUT, "SessionStageProxyLayoutTest: UpdateWindowModeForUITest01 end");
}

/**
 * @tc.name: NotifyGlobalScaledRectChange
 * @tc.desc: test function : NotifyGlobalScaledRectChange
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, NotifyGlobalScaledRectChange, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    GTEST_LOG_(INFO) << "SessionStageProxyLayoutTest: NotifyGlobalScaledRectChange start";
    auto remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto sessionStageProxy = sptr<SessionStageProxy>::MakeSptr(remoteMocker);
    auto sessionStageProxyNull = sptr<SessionStageProxy>::MakeSptr(nullptr);
    Rect globalScaledRect = { 0, 0, 1000, 1000 };

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    sessionStageProxy->NotifyGlobalScaledRectChange(globalScaledRect);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") != std::string::npos);
    g_logMsg.clear();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    MockMessageParcel::SetWriteInt32ErrorFlag(true);
    sessionStageProxy->NotifyGlobalScaledRectChange(globalScaledRect);
    EXPECT_TRUE(g_logMsg.find("write globalScaledRect failed") != std::string::npos);
    g_logMsg.clear();
    MockMessageParcel::SetWriteInt32ErrorFlag(false);

    sessionStageProxyNull->NotifyGlobalScaledRectChange(globalScaledRect);
    EXPECT_TRUE(g_logMsg.find("remote is null") != std::string::npos);
    g_logMsg.clear();

    remoteMocker->SetRequestResult(1);
    sessionStageProxy->NotifyGlobalScaledRectChange(globalScaledRect);
    EXPECT_TRUE(g_logMsg.find("SendRequest failed") != std::string::npos);
    g_logMsg.clear();
    remoteMocker->SetRequestResult(ERR_NONE);

    sessionStageProxy->NotifyGlobalScaledRectChange(globalScaledRect);
    EXPECT_TRUE(g_logMsg.find("WriteInterfaceToken failed") == std::string::npos);
    EXPECT_TRUE(g_logMsg.find("write globalScaledRect failed") == std::string::npos);
    EXPECT_TRUE(g_logMsg.find("remote is null") == std::string::npos);
    EXPECT_TRUE(g_logMsg.find("SendRequest failed") == std::string::npos);

    g_logMsg.clear();
    MockMessageParcel::ClearAllErrorFlag();
    GTEST_LOG_(INFO) << "SessionStageProxyLayoutTest: NotifyGlobalScaledRectChange end";
}

/**
 * @tc.name: UpdateAttachedWindowLimits01
 * @tc.desc: Test UpdateAttachedWindowLimits with valid limits
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, UpdateAttachedWindowLimits01, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    int32_t sourcePersistentId = 1001;
    WindowLimits attachedLimits = { 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    bool isIntersectedHeightLimit = true;
    bool isIntersectedWidthLimit = true;

    WSError res = sessionStage_->UpdateAttachedWindowLimits(sourcePersistentId, attachedLimits,
        isIntersectedHeightLimit, isIntersectedWidthLimit);
    EXPECT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: UpdateAttachedWindowLimits02
 * @tc.desc: Test UpdateAttachedWindowLimits with WriteInterfaceToken error
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, UpdateAttachedWindowLimits02, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);

    int32_t sourcePersistentId = 1002;
    WindowLimits attachedLimits = { 1800, 900, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WSError res = sessionStage_->UpdateAttachedWindowLimits(sourcePersistentId, attachedLimits, false, true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
}

/**
 * @tc.name: UpdateAttachedWindowLimits03
 * @tc.desc: Test UpdateAttachedWindowLimits with VP unit limits
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, UpdateAttachedWindowLimits03, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    int32_t sourcePersistentId = 1003;
    WindowLimits attachedLimits = { 1000, 500, 50, 100, 0.0f, 0.0f, 0.0f, PixelUnit::VP };

    WSError res = sessionStage_->UpdateAttachedWindowLimits(sourcePersistentId, attachedLimits, true, false);
    EXPECT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: UpdateAttachedWindowLimits04
 * @tc.desc: Test UpdateAttachedWindowLimits with null remote object
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, UpdateAttachedWindowLimits04, TestSize.Level1)
{
    // Create proxy with null remote object
    auto sessionStage = sptr<SessionStageProxy>::MakeSptr(nullptr);

    int32_t sourcePersistentId = 1004;
    WindowLimits attachedLimits = { 1600, 800, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WSError res = sessionStage->UpdateAttachedWindowLimits(sourcePersistentId, attachedLimits, true, true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: UpdateAttachedWindowLimits05
 * @tc.desc: Test UpdateAttachedWindowLimits with WriteInt32 error
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, UpdateAttachedWindowLimits05, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    MockMessageParcel::SetWriteInt32ErrorFlag(true);

    int32_t sourcePersistentId = 1005;
    WindowLimits attachedLimits = { 1700, 900, 120, 220, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WSError res = sessionStage_->UpdateAttachedWindowLimits(sourcePersistentId, attachedLimits, true, false);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    MockMessageParcel::SetWriteInt32ErrorFlag(false);
}

/**
 * @tc.name: UpdateAttachedWindowLimits06
 * @tc.desc: Test UpdateAttachedWindowLimits with Marshalling error
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, UpdateAttachedWindowLimits06, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    MockMessageParcel::SetWriteUint32ErrorFlag(true);

    int32_t sourcePersistentId = 1006;
    WindowLimits attachedLimits = { 1750, 950, 130, 230, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WSError res = sessionStage_->UpdateAttachedWindowLimits(sourcePersistentId, attachedLimits, false, true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    MockMessageParcel::SetWriteUint32ErrorFlag(false);
}

/**
 * @tc.name: UpdateAttachedWindowLimits07
 * @tc.desc: Test UpdateAttachedWindowLimits with WriteBool(isIntersectedHeightLimit) error
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, UpdateAttachedWindowLimits07, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    MockMessageParcel::SetWriteBoolErrorCount(0);

    int32_t sourcePersistentId = 1007;
    WindowLimits attachedLimits = { 1800, 980, 140, 240, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WSError res = sessionStage_->UpdateAttachedWindowLimits(sourcePersistentId, attachedLimits, true, true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: UpdateAttachedWindowLimits08
 * @tc.desc: Test UpdateAttachedWindowLimits with WriteBool(isIntersectedWidthLimit) error
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, UpdateAttachedWindowLimits08, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    MockMessageParcel::SetWriteBoolErrorCount(1);

    int32_t sourcePersistentId = 1008;
    WindowLimits attachedLimits = { 1850, 970, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WSError res = sessionStage_->UpdateAttachedWindowLimits(sourcePersistentId, attachedLimits, true, true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: UpdateAttachedWindowLimits09
 * @tc.desc: Test UpdateAttachedWindowLimits with SendRequest error
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, UpdateAttachedWindowLimits09, TestSize.Level1)
{
    auto remoteMock = sptr<MockIRemoteObject>::MakeSptr();
    remoteMock->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionStageProxy> failSendProxy = sptr<SessionStageProxy>::MakeSptr(remoteMock);

    int32_t sourcePersistentId = 1009;
    WindowLimits attachedLimits = { 1900, 990, 160, 260, 0.0f, 0.0f, 0.0f, PixelUnit::PX };
    WSError res = failSendProxy->UpdateAttachedWindowLimits(sourcePersistentId, attachedLimits, true, true);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: RemoveAttachedWindowLimits01
 * @tc.desc: Test RemoveAttachedWindowLimits with valid sourceId
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, RemoveAttachedWindowLimits01, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    int32_t sourcePersistentId = 2001;

    WSError res = sessionStage_->RemoveAttachedWindowLimits(sourcePersistentId);
    EXPECT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: RemoveAttachedWindowLimits02
 * @tc.desc: Test RemoveAttachedWindowLimits with WriteInterfaceToken error
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, RemoveAttachedWindowLimits02, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);

    int32_t sourcePersistentId = 2002;
    WSError res = sessionStage_->RemoveAttachedWindowLimits(sourcePersistentId);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
}

/**
 * @tc.name: RemoveAttachedWindowLimits03
 * @tc.desc: Test RemoveAttachedWindowLimits with null remote object
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, RemoveAttachedWindowLimits03, TestSize.Level1)
{
    // Create proxy with null remote object
    auto sessionStage = sptr<SessionStageProxy>::MakeSptr(nullptr);

    int32_t sourcePersistentId = 2003;
    WSError res = sessionStage->RemoveAttachedWindowLimits(sourcePersistentId);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: RemoveAttachedWindowLimits04
 * @tc.desc: Test RemoveAttachedWindowLimits with WriteInt32 error
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, RemoveAttachedWindowLimits04, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    MockMessageParcel::SetWriteInt32ErrorFlag(true);

    int32_t sourcePersistentId = 2004;
    WSError res = sessionStage_->RemoveAttachedWindowLimits(sourcePersistentId);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    MockMessageParcel::SetWriteInt32ErrorFlag(false);
}

/**
 * @tc.name: RemoveAttachedWindowLimits05
 * @tc.desc: Test RemoveAttachedWindowLimits with SendRequest error
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, RemoveAttachedWindowLimits05, TestSize.Level1)
{
    auto remoteMock = sptr<MockIRemoteObject>::MakeSptr();
    remoteMock->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionStageProxy> failSendProxy = sptr<SessionStageProxy>::MakeSptr(remoteMock);

    int32_t sourcePersistentId = 2005;
    WSError res = failSendProxy->RemoveAttachedWindowLimits(sourcePersistentId);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: UpdateAppHookWindowInfo
 * @tc.desc: test function : UpdateAppHookWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, UpdateAppHookWindowInfo, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionStageProxyLayoutTest: UpdateAppHookWindowInfo start";
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    sptr<SessionStageProxy> sessionStageProxy = sptr<SessionStageProxy>::MakeSptr(remoteMocker);
    HookWindowInfo hookWindowInfo;

    // Case 1: Failed to write interface token
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    WSError errCode = sessionStageProxy->UpdateAppHookWindowInfo(hookWindowInfo);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);

    // Case 2: remote is nullptr
    sptr<SessionStageProxy> nullProxy = sptr<SessionStageProxy>::MakeSptr(nullptr);
    errCode = nullProxy->UpdateAppHookWindowInfo(hookWindowInfo);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);

    // Case 3: Failed to write hook info
    MockMessageParcel::SetWriteParcelableErrorFlag(true);
    errCode = sessionStageProxy->UpdateAppHookWindowInfo(hookWindowInfo);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    MockMessageParcel::SetWriteParcelableErrorFlag(false);

    // Case 4: Failed to send request
    remoteMocker->SetRequestResult(ERR_TRANSACTION_FAILED);
    errCode = sessionStageProxy->UpdateAppHookWindowInfo(hookWindowInfo);
    EXPECT_EQ(errCode, WSError::WS_ERROR_IPC_FAILED);
    remoteMocker->SetRequestResult(ERR_NONE);

    // Case 5: Success
    errCode = sessionStageProxy->UpdateAppHookWindowInfo(hookWindowInfo);
    MockMessageParcel::SetReadInt32ErrorFlag(false);
    EXPECT_EQ(errCode, WSError::WS_OK);

    MockMessageParcel::ClearAllErrorFlag();
    GTEST_LOG_(INFO) << "SessionStageProxyLayoutTest: UpdateAppHookWindowInfo end";
}
/**
 * @tc.name: SyncAllAttachedLimitsToChild01
 * @tc.desc: Test SyncAllAttachedLimitsToChild with valid lists
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, SyncAllAttachedLimitsToChild01, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    std::vector<std::pair<int32_t, WindowLimits>> limitsList;
    std::vector<std::pair<int32_t, AttachLimitOptions>> optionsList;

    limitsList.emplace_back(1001, WindowLimits{ 2000, 1000, 200, 300, 0.0f, 0.0f, 0.0f, PixelUnit::PX });
    optionsList.emplace_back(1001, AttachLimitOptions{ true, true });

    limitsList.emplace_back(1002, WindowLimits{ 1800, 900, 150, 250, 0.0f, 0.0f, 0.0f, PixelUnit::PX });
    optionsList.emplace_back(1002, AttachLimitOptions{ true, false });

    WSError res = sessionStage_->SyncAllAttachedLimitsToChild(limitsList, optionsList);
    EXPECT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: SyncAllAttachedLimitsToChild02
 * @tc.desc: Test SyncAllAttachedLimitsToChild with empty lists
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, SyncAllAttachedLimitsToChild02, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    std::vector<std::pair<int32_t, WindowLimits>> limitsList;
    std::vector<std::pair<int32_t, AttachLimitOptions>> optionsList;

    WSError res = sessionStage_->SyncAllAttachedLimitsToChild(limitsList, optionsList);
    EXPECT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: SyncAllAttachedLimitsToChild03
 * @tc.desc: Test SyncAllAttachedLimitsToChild with WriteInterfaceToken error
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, SyncAllAttachedLimitsToChild03, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);

    std::vector<std::pair<int32_t, WindowLimits>> limitsList;
    std::vector<std::pair<int32_t, AttachLimitOptions>> optionsList;
    limitsList.emplace_back(1003, WindowLimits{ 1600, 800, 100, 200, 0.0f, 0.0f, 0.0f, PixelUnit::PX });
    optionsList.emplace_back(1003, AttachLimitOptions{ false, true });

    WSError res = sessionStage_->SyncAllAttachedLimitsToChild(limitsList, optionsList);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
}

/**
 * @tc.name: SyncAllAttachedLimitsToChild04
 * @tc.desc: Test SyncAllAttachedLimitsToChild with null remote object
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, SyncAllAttachedLimitsToChild04, TestSize.Level1)
{
    auto sessionStage = sptr<SessionStageProxy>::MakeSptr(nullptr);

    std::vector<std::pair<int32_t, WindowLimits>> limitsList;
    std::vector<std::pair<int32_t, AttachLimitOptions>> optionsList;
    limitsList.emplace_back(1004, WindowLimits{ 1700, 850, 120, 220, 0.0f, 0.0f, 0.0f, PixelUnit::PX });
    optionsList.emplace_back(1004, AttachLimitOptions{ true, true });

    WSError res = sessionStage->SyncAllAttachedLimitsToChild(limitsList, optionsList);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

/**
 * @tc.name: SyncAllAttachedLimitsToChild05
 * @tc.desc: Test SyncAllAttachedLimitsToChild with WriteUint32 error
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, SyncAllAttachedLimitsToChild05, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    MockMessageParcel::SetWriteUint32ErrorFlag(true);

    std::vector<std::pair<int32_t, WindowLimits>> limitsList;
    std::vector<std::pair<int32_t, AttachLimitOptions>> optionsList;
    limitsList.emplace_back(1005, WindowLimits{ 1750, 950, 130, 230, 0.0f, 0.0f, 0.0f, PixelUnit::PX });
    optionsList.emplace_back(1005, AttachLimitOptions{ true, false });

    WSError res = sessionStage_->SyncAllAttachedLimitsToChild(limitsList, optionsList);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    MockMessageParcel::SetWriteUint32ErrorFlag(false);
}

/**
 * @tc.name: SyncAllAttachedLimitsToChild06
 * @tc.desc: Test SyncAllAttachedLimitsToChild with WriteInt32 error
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, SyncAllAttachedLimitsToChild06, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    MockMessageParcel::SetWriteInt32ErrorFlag(true);

    std::vector<std::pair<int32_t, WindowLimits>> limitsList;
    std::vector<std::pair<int32_t, AttachLimitOptions>> optionsList;
    limitsList.emplace_back(1006, WindowLimits{ 1800, 980, 140, 240, 0.0f, 0.0f, 0.0f, PixelUnit::PX });
    optionsList.emplace_back(1006, AttachLimitOptions{ false, true });

    WSError res = sessionStage_->SyncAllAttachedLimitsToChild(limitsList, optionsList);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    MockMessageParcel::SetWriteInt32ErrorFlag(false);
}

/**
 * @tc.name: SyncAllAttachedLimitsToChild07
 * @tc.desc: Test SyncAllAttachedLimitsToChild with WriteBool error
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, SyncAllAttachedLimitsToChild07, TestSize.Level1)
{
    ASSERT_TRUE((sessionStage_ != nullptr));
    MockMessageParcel::SetWriteBoolErrorFlag(true);
    MockMessageParcel::SetWriteBoolErrorCount(0);

    std::vector<std::pair<int32_t, WindowLimits>> limitsList;
    std::vector<std::pair<int32_t, AttachLimitOptions>> optionsList;
    limitsList.emplace_back(1007, WindowLimits{ 1900, 990, 160, 260, 0.0f, 0.0f, 0.0f, PixelUnit::PX });
    optionsList.emplace_back(1007, AttachLimitOptions{ true, true });

    WSError res = sessionStage_->SyncAllAttachedLimitsToChild(limitsList, optionsList);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);

    MockMessageParcel::ClearAllErrorFlag();
}

/**
 * @tc.name: SyncAllAttachedLimitsToChild08
 * @tc.desc: Test SyncAllAttachedLimitsToChild with SendRequest error
 * @tc.type: FUNC
 */
HWTEST_F(SessionStageProxyLayoutTest, SyncAllAttachedLimitsToChild08, TestSize.Level1)
{
    auto remoteMock = sptr<MockIRemoteObject>::MakeSptr();
    remoteMock->sendRequestResult_ = ERR_TRANSACTION_FAILED;
    sptr<SessionStageProxy> failSendProxy = sptr<SessionStageProxy>::MakeSptr(remoteMock);

    std::vector<std::pair<int32_t, WindowLimits>> limitsList;
    std::vector<std::pair<int32_t, AttachLimitOptions>> optionsList;
    limitsList.emplace_back(1008, WindowLimits{ 1950, 995, 170, 270, 0.0f, 0.0f, 0.0f, PixelUnit::PX });
    optionsList.emplace_back(1008, AttachLimitOptions{ true, true });

    WSError res = failSendProxy->SyncAllAttachedLimitsToChild(limitsList, optionsList);
    EXPECT_EQ(WSError::WS_ERROR_IPC_FAILED, res);
}

} // namespace
} // namespace Rosen
} // namespace OHOS