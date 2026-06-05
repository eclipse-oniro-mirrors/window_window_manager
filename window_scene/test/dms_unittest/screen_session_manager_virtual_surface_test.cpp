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
#include <common/rs_rect.h>
#include <transaction/rs_marshalling_helper.h>
#include <iremote_object_mocker.h>

#include "screen_session_manager/include/screen_session_manager.h"
#include "display_manager_agent_default.h"
#include "iconsumer_surface.h"
#include "scene_board_judgement.h"
#include "session/screen/include/screen_session.h"
#include "zidl/screen_session_manager_proxy.h"
#include "zidl/screen_session_manager_stub.h"
#include "common_test_utils.h"
#include "mock/mock_accesstoken_kit.h"
#include "mock_message_parcel.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_IN_US = 100000;
const ScreenId DEFAULT_SCREEN_ID = 0;
const ScreenId VIRTUAL_SCREEN_ID = 2;
const ScreenId VIRTUAL_SCREEN_RS_ID = 100;
std::string g_logMsg;

void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
    const char *msg)
{
    g_logMsg = msg;
}
}

class ScreenSessionManagerVirtualSurfaceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<ScreenSessionManager> ssm_;
    ScreenId DEFAULT_SCREEN_ID_TEST = 0;
    ScreenId VIRTUAL_SCREEN_ID_TEST = 2;
};

sptr<ScreenSessionManager> ScreenSessionManagerVirtualSurfaceTest::ssm_ = nullptr;

void ScreenSessionManagerVirtualSurfaceTest::SetUpTestCase()
{
    ssm_ = new (std::nothrow) ScreenSessionManager();
    CommonTestUtils::InjectTokenInfoByHapName(0, "com.ohos.systemui", 0);
    const char** perms = new (std::nothrow) const char *[1];
    perms[0] = "ohos.permission.CAPTURE_SCREEN";
    CommonTestUtils::SetAceessTokenPermission("foundation", perms, 1);
    delete[] perms;
}

void ScreenSessionManagerVirtualSurfaceTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void ScreenSessionManagerVirtualSurfaceTest::SetUp()
{
}

void ScreenSessionManagerVirtualSurfaceTest::TearDown()
{
    usleep(SLEEP_TIME_IN_US);
}

namespace {
/**
 * @tc.name: AddVirtualScreenSurface_SurfaceIsNull
 * @tc.desc: AddVirtualScreenSurface_SurfaceIsNull test
 * @tc.type: FUNC
 */ 
HWTEST_F(ScreenSessionManagerVirtualSurfaceTest, AddVirtualScreenSurface_SurfaceIsNull, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new (std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "testAddVirtualScreenSurface01";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());

    std::vector<ScreenId> mirrorScreenIds;
    ScreenId mainScreenId(DEFAULT_SCREEN_ID_TEST);
    ScreenId screenGroupId = 1;
    mirrorScreenIds.push_back(screenId);
    ssm_->MakeMirror(mainScreenId, mirrorScreenIds, screenGroupId);

    DMRect surfaceRegion = {0, 0, 100, 100};
    auto result = ssm_->AddVirtualScreenSurface(screenId, nullptr, surfaceRegion);
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, result);

    ssm_->DestroyVirtualScreen(screenId);
}

/**
 * @tc.name: AddVirtualScreenSurface_RemoteIsNull
 * @tc.desc: RemoveVirtualScreenSurface_SurfaceIsNull test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerVirtualSurfaceTest, RemoveVirtualScreenSurface_SurfaceIsNull, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IDisplayManagerAgent> displayManagerAgent = new (std::nothrow) DisplayManagerAgentDefault();
    VirtualScreenOption virtualOption;
    virtualOption.name_ = "testRemoveVirtualScreenSurface01";
    auto screenId = ssm_->CreateVirtualScreen(virtualOption, displayManagerAgent->AsObject());

    std::vector<ScreenId> mirrorScreenIds;
    ScreenId mainScreenId(DEFAULT_SCREEN_ID_TEST);
    ScreenId screenGroupId = 1;
    mirrorScreenIds.push_back(screenId);
    ssm_->MakeMirror(mainScreenId, mirrorScreenIds, screenGroupId);

    auto result = ssm_->RemoveVirtualScreenSurface(screenId, nullptr);
    ASSERT_EQ(DMError::DM_ERROR_INVALID_PARAM, result);

    ssm_->DestroyVirtualScreen(screenId);
}
}

class ScreenSessionManagerProxyVirtualSurfaceTest : public testing::Test {
public:
    static void SetUpTestSuite();
    void SetUp() override;
    sptr<ScreenSessionManagerProxy> screenSessionManagerProxy;
};

void ScreenSessionManagerProxyVirtualSurfaceTest::SetUpTestSuite()
{
}

void ScreenSessionManagerProxyVirtualSurfaceTest::SetUp()
{
    if (screenSessionManagerProxy) {
        return;
    }

    sptr<IRemoteObject> impl = sptr<IRemoteObjectMocker>::MakeSptr();
    screenSessionManagerProxy = sptr<ScreenSessionManagerProxy>::MakeSptr(impl);
}

namespace {
/**
 * @tc.name: AddVirtualScreenSurface_RemoteIsNull
 * @tc.desc: AddVirtualScreenSurface_RemoteIsNull test
 * @tc.type: FUNC
 */  
HWTEST_F(ScreenSessionManagerProxyVirtualSurfaceTest, AddVirtualScreenSurface_RemoteIsNull, TestSize.Level1)
{
    auto proxy = sptr<ScreenSessionManagerProxy>::MakeSptr(nullptr);
    ScreenId screenId = 1001;
    sptr<IConsumerSurface> surface = IConsumerSurface::Create();
    DMRect surfaceRegion = {0, 0, 100, 100};

    auto result = proxy->AddVirtualScreenSurface(screenId, surface->GetProducer(), surfaceRegion);
    ASSERT_EQ(DMError::DM_ERROR_REMOTE_CREATE_FAILED, result);
}

/**
 * @tc.name: AddVirtualScreenSurface_WriteInterfaceTokenFailed
 * @tc.desc: AddVirtualScreenSurface_WriteInterfaceTokenFailed test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyVirtualSurfaceTest,
    AddVirtualScreenSurface_WriteInterfaceTokenFailed, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<ScreenSessionManagerProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ScreenId screenId = 1001;
    sptr<IConsumerSurface> surface = IConsumerSurface::Create();
    DMRect surfaceRegion = {0, 0, 100, 100};
    
    auto result = proxy->AddVirtualScreenSurface(screenId, surface->GetProducer(), surfaceRegion);
    ASSERT_EQ(DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED, result);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
}

/**
 * @tc.name: AddVirtualScreenSurface_WriteFailed
 * @tc.desc: AddVirtualScreenSurface_WriteFailed test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyVirtualSurfaceTest, AddVirtualScreenSurface_WriteFailed, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<ScreenSessionManagerProxy>::MakeSptr(remoteMocker);
    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    ScreenId screenId = 1001;
    sptr<IConsumerSurface> surface = IConsumerSurface::Create();
    DMRect surfaceRegion = {0, 0, 100, 100};
    
    auto result = proxy->AddVirtualScreenSurface(screenId, surface->GetProducer(), surfaceRegion);
    ASSERT_EQ(DMError::DM_ERROR_IPC_FAILED, result);
    MockMessageParcel::SetWriteUint64ErrorFlag(false);
}

/**
 * @tc.name: AddVirtualScreenSurface_SendRequestFailed
 * @tc.desc: AddVirtualScreenSurface_SendRequestFailed test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyVirtualSurfaceTest, AddVirtualScreenSurface_SendRequestFailed, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<ScreenSessionManagerProxy>::MakeSptr(remoteMocker);
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ScreenId screenId = 1001;
    sptr<IConsumerSurface> surface = IConsumerSurface::Create();
    DMRect surfaceRegion = {0, 0, 100, 100};
    
    auto result = proxy->AddVirtualScreenSurface(screenId, surface->GetProducer(), surfaceRegion);
    ASSERT_EQ(DMError::DM_ERROR_IPC_FAILED, result);
    remoteMocker->SetRequestResult(ERR_NONE);
}

/**
 * @tc.name: AddVirtualScreenSurface_SurfaceIsNull
 * @tc.desc: AddVirtualScreenSurface_SurfaceIsNull test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyVirtualSurfaceTest, AddVirtualScreenSurface_SurfaceIsNull, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<ScreenSessionManagerProxy>::MakeSptr(remoteMocker);
    
    ScreenId screenId = 1001;
    DMRect surfaceRegion = {0, 0, 100, 100};
    
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto result = proxy->AddVirtualScreenSurface(screenId, nullptr, surfaceRegion);
    EXPECT_TRUE(g_logMsg.find("surface is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: AddVirtualScreenSurface_Success
 * @tc.desc: AddVirtualScreenSurface_Success test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyVirtualSurfaceTest, AddVirtualScreenSurface_Success, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<ScreenSessionManagerProxy>::MakeSptr(remoteMocker);
    
    ScreenId screenId = 1001;
    sptr<IConsumerSurface> surface = IConsumerSurface::Create();
    DMRect surfaceRegion = {0, 0, 100, 100};
    
    auto result = proxy->AddVirtualScreenSurface(screenId, surface->GetProducer(), surfaceRegion);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(result, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(result, DMError::DM_ERROR_IPC_FAILED);
    }
}

/**
 * @tc.name: RemoveVirtualScreenSurface_RemoteIsNull
 * @tc.desc: RemoveVirtualScreenSurface_RemoteIsNull test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyVirtualSurfaceTest, RemoveVirtualScreenSurface_RemoteIsNull, TestSize.Level1)
{
    auto proxy = sptr<ScreenSessionManagerProxy>::MakeSptr(nullptr);
    ScreenId screenId = 1001;
    sptr<IConsumerSurface> surface = IConsumerSurface::Create();
    
    auto result = proxy->RemoveVirtualScreenSurface(screenId, surface->GetProducer());
    ASSERT_EQ(DMError::DM_ERROR_REMOTE_CREATE_FAILED, result);
}

/**
 * @tc.name: RemoveVirtualScreenSurface_WriteInterfaceTokenFailed
 * @tc.desc: RemoveVirtualScreenSurface_WriteInterfaceTokenFailed test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyVirtualSurfaceTest, RemoveVirtualScreenSurface_WriteInterfaceTokenFailed,
    TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<ScreenSessionManagerProxy>::MakeSptr(remoteMocker);
    
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    ScreenId screenId = 1001;
    sptr<IConsumerSurface> surface = IConsumerSurface::Create();
    
    auto result = proxy->RemoveVirtualScreenSurface(screenId, surface->GetProducer());
    ASSERT_EQ(DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED, result);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
}

/**
 * @tc.name: RemoveVirtualScreenSurface_WriteFailed
 * @tc.desc: RemoveVirtualScreenSurface_WriteFailed test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyVirtualSurfaceTest, RemoveVirtualScreenSurface_WriteFailed, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<ScreenSessionManagerProxy>::MakeSptr(remoteMocker);
    
    MockMessageParcel::SetWriteUint64ErrorFlag(true);
    ScreenId screenId = 1001;
    sptr<IConsumerSurface> surface = IConsumerSurface::Create();
    
    auto result = proxy->RemoveVirtualScreenSurface(screenId, surface->GetProducer());
    ASSERT_EQ(DMError::DM_ERROR_IPC_FAILED, result);
    MockMessageParcel::SetWriteUint64ErrorFlag(false);
}

/**
 * @tc.name: RemoveVirtualScreenSurface_SendRequestFailed
 * @tc.desc: RemoveVirtualScreenSurface_SendRequestFailed test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyVirtualSurfaceTest, RemoveVirtualScreenSurface_SendRequestFailed, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<ScreenSessionManagerProxy>::MakeSptr(remoteMocker);
    
    remoteMocker->SetRequestResult(ERR_INVALID_DATA);
    ScreenId screenId = 1001;
    sptr<IConsumerSurface> surface = IConsumerSurface::Create();
    
    auto result = proxy->RemoveVirtualScreenSurface(screenId, surface->GetProducer());
    ASSERT_EQ(DMError::DM_ERROR_IPC_FAILED, result);
    remoteMocker->SetRequestResult(ERR_NONE);
}

/**
 * @tc.name: RemoveVirtualScreenSurface_SurfaceIsNull
 * @tc.desc: RemoveVirtualScreenSurface_SurfaceIsNull test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyVirtualSurfaceTest, RemoveVirtualScreenSurface_SurfaceIsNull, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<ScreenSessionManagerProxy>::MakeSptr(remoteMocker);
    
    ScreenId screenId = 1001;
    
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto result = proxy->RemoveVirtualScreenSurface(screenId, nullptr);
    EXPECT_TRUE(g_logMsg.find("surface is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: RemoveVirtualScreenSurface_Success
 * @tc.desc: RemoveVirtualScreenSurface_Success test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerProxyVirtualSurfaceTest, RemoveVirtualScreenSurface_Success, TestSize.Level1)
{
    MockMessageParcel::ClearAllErrorFlag();
    sptr<MockIRemoteObject> remoteMocker = sptr<MockIRemoteObject>::MakeSptr();
    auto proxy = sptr<ScreenSessionManagerProxy>::MakeSptr(remoteMocker);
    
    ScreenId screenId = 1001;
    sptr<IConsumerSurface> surface = IConsumerSurface::Create();
    
    auto result = proxy->RemoveVirtualScreenSurface(screenId, surface->GetProducer());
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_NE(result, DMError::DM_ERROR_IPC_FAILED);
    } else {
        ASSERT_EQ(result, DMError::DM_ERROR_IPC_FAILED);
    }
}
}
class ScreenSessionManagerStubVirtualSurfaceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<ScreenSessionManagerStub> stub_;
};

void ScreenSessionManagerStubVirtualSurfaceTest::SetUpTestCase()
{
}

void ScreenSessionManagerStubVirtualSurfaceTest::TearDownTestCase()
{
    usleep(SLEEP_TIME_IN_US);
}

void ScreenSessionManagerStubVirtualSurfaceTest::SetUp()
{
    stub_ = new (std::nothrow) ScreenSessionManagerStub();
}

void ScreenSessionManagerStubVirtualSurfaceTest::TearDown()
{
}

namespace {
/**
 * @tc.name: OnRemoteRequest_AddVirtualScreenSurface_InvalidToken
 * @tc.desc: OnRemoteRequest_AddVirtualScreenSurface_InvalidToken test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubVirtualSurfaceTest,
    OnRemoteRequest_AddVirtualScreenSurface_InvalidToken, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(u"InvalidToken");
    ScreenId screenId = 1001;
    data.WriteUint64(static_cast<uint64_t>(screenId));

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_ADD_VIRTUAL_SCREEN_SURFACE);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(res, 0);
}

/**
 * @tc.name: OnRemoteRequest_RemoveVirtualScreenSurface_InvalidToken
 * @tc.desc: OnRemoteRequest_RemoveVirtualScreenSurface_InvalidToken test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionManagerStubVirtualSurfaceTest, OnRemoteRequest_RemoveVirtualScreenSurface_InvalidToken,
    TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(u"InvalidToken");
    ScreenId screenId = 1001;
    data.WriteUint64(static_cast<uint64_t>(screenId));

    uint32_t code = static_cast<uint32_t>(
        DisplayManagerMessage::TRANS_ID_REMOVE_VIRTUAL_SCREEN_SURFACE);

    int res = stub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(res, 0);
}
}
}
}