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

#include <condition_variable>
#include <mutex>
#include <chrono>

#include "connection/screen_session_ability_connection.h"
#include "extension_manager_client.h"
#include "ipc_skeleton.h"
#include "scene_board_judgement.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
constexpr int32_t DEFAULT_VALUE = -1;
constexpr uint32_t EXTENSION_CONNECT_OUT_TIME = 300; // ms
constexpr uint32_t TRANS_CMD_SEND_SNAPSHOT_RECT = 2;
constexpr int32_t RES_FAILURE = -1;
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000;
}

class ScreenSessionAbilityConnectionUtTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::recursive_mutex mutex_;

private:
    std::mutex connectedMutex_;
    std::condition_variable connectedCv_;
};

void ScreenSessionAbilityConnectionUtTest::SetUpTestCase()
{
}

void ScreenSessionAbilityConnectionUtTest::TearDownTestCase()
{
}

void ScreenSessionAbilityConnectionUtTest::SetUp()
{
}

void ScreenSessionAbilityConnectionUtTest::TearDown()
{
    usleep(SLEEP_TIME_US);
}

namespace {

/**
 * @tc.name: OnAbilityConnectDone
 * @tc.desc: OnAbilityConnectDone func test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, OnAbilityConnectDone, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    AAFwk::Want want;
    want.SetElementName("com.ohos.sceneboard",
        "com.ohos.sceneboard.systemdialog");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto resConnect = AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(
        want, abilityConnectionStub, nullptr, DEFAULT_VALUE);
    IPCSkeleton::SetCallingIdentity(identity);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(resConnect, ERR_OK);
        {
            std::unique_lock<std::mutex> lock(connectedMutex_);
            static_cast<void>(connectedCv_.wait_for(lock, std::chrono::milliseconds(EXTENSION_CONNECT_OUT_TIME)));
        }
        auto resDisconnect = AAFwk::ExtensionManagerClient::GetInstance().DisconnectAbility(abilityConnectionStub);
        ASSERT_EQ(resDisconnect, NO_ERROR);
    } else {
        ASSERT_NE(resConnect, ERR_OK);
    }
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: OnAbilityDisconnectDone
 * @tc.desc: OnAbilityDisconnectDone func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, OnAbilityDisconnectDone, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    AAFwk::Want want;
    want.SetElementName("com.ohos.sceneboard",
        "com.ohos.sceneboard.systemdialog");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto resConnect = AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(
        want, abilityConnectionStub, nullptr, DEFAULT_VALUE);
    IPCSkeleton::SetCallingIdentity(identity);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(resConnect, ERR_OK);
        AAFwk::ExtensionManagerClient::GetInstance().DisconnectAbility(abilityConnectionStub);
        EXPECT_EQ(abilityConnectionStub->IsAbilityConnected(), false);
    } else {
        ASSERT_NE(resConnect, ERR_OK);
    }
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: AddObjectDeathRecipient
 * @tc.desc: AddObjectDeathRecipient func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, AddObjectDeathRecipient, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    EXPECT_EQ(abilityConnectionStub->AddObjectDeathRecipient(), false);
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: SendMessage
 * @tc.desc: SendMessage func test02
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, SendMessage02, Function | SmallTest | Level1)
{
    ScreenSessionAbilityConnection screenSessionAbilityConnection;
    int32_t transCode = 0;
    MessageParcel data;
    MessageParcel reply;
    auto ret = screenSessionAbilityConnection.SendMessage(transCode, data, reply);
    EXPECT_EQ(ret, RES_FAILURE);
}

/**
 * @tc.name: IsConnected
 * @tc.desc: IsConnected func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, IsConnected, Function | SmallTest | Level1)
{
    ScreenSessionAbilityConnection screenSessionAbilityConnection;
    auto ret = screenSessionAbilityConnection.IsConnected();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: IsAbilityConnected
 * @tc.desc: IsAbilityConnected func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, IsAbilityConnected, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    EXPECT_EQ(abilityConnectionStub->IsAbilityConnected(), false);
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: SendMessageSync
 * @tc.desc: SendMessageSync func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, SendMessageSync, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    AAFwk::Want want;
    want.SetElementName("com.ohos.sceneboard",
        "com.ohos.sceneboard.systemdialog");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto resConnect = AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(
        want, abilityConnectionStub, nullptr, DEFAULT_VALUE);
    IPCSkeleton::SetCallingIdentity(identity);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(resConnect, ERR_OK);
        {
            std::unique_lock<std::mutex> lock(connectedMutex_);
            static_cast<void>(connectedCv_.wait_for(lock, std::chrono::milliseconds(EXTENSION_CONNECT_OUT_TIME)));
        }
        MessageParcel data;
        MessageParcel reply;
        data.WriteString16(Str8ToStr16("SA"));
        data.WriteString16(Str8ToStr16("ScreenSessionManager"));
        abilityConnectionStub->SendMessageSync(TRANS_CMD_SEND_SNAPSHOT_RECT, data, reply);
        EXPECT_EQ(reply.ReadInt32(), 0);
        EXPECT_EQ(reply.ReadInt32(), 0);
        EXPECT_EQ(reply.ReadInt32(), 0);
        EXPECT_EQ(reply.ReadInt32(), 0);
        EXPECT_EQ(reply.ReadInt32(), 0);
        auto resDisconnect = AAFwk::ExtensionManagerClient::GetInstance().DisconnectAbility(abilityConnectionStub);
        ASSERT_EQ(resDisconnect, NO_ERROR);
    } else {
        ASSERT_NE(resConnect, ERR_OK);
    }
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: OnRemoteDied
 * @tc.desc: OnRemoteDied func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, OnRemoteDied, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    AAFwk::Want want;
    want.SetElementName("com.ohos.sceneboard",
        "com.ohos.sceneboard.systemdialog");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto resConnect = AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(
        want, abilityConnectionStub, nullptr, DEFAULT_VALUE);
    IPCSkeleton::SetCallingIdentity(identity);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(resConnect, ERR_OK);
        AAFwk::ExtensionManagerClient::GetInstance().DisconnectAbility(abilityConnectionStub);
        EXPECT_EQ(abilityConnectionStub->IsAbilityConnected(), false);
    } else {
        ASSERT_NE(resConnect, ERR_OK);
    }
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: IsAbilityConnectedSync
 * @tc.desc: IsAbilityConnectedSync func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, IsAbilityConnectedSync, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    EXPECT_EQ(abilityConnectionStub->IsAbilityConnectedSync(), false);
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: ScreenSessionConnectExtension
 * @tc.desc: ScreenSessionConnectExtension func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, ScreenSessionConnectExtension, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    AAFwk::Want want;
    const std::string bundleName = "com.ohos.sceneboard";
    const std::string abilityName = "com.ohos.sceneboard.systemdialog";
    want.SetElementName(bundleName, abilityName);
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto resConnect = AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(
        want, abilityConnectionStub, nullptr, DEFAULT_VALUE);
    IPCSkeleton::SetCallingIdentity(identity);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(resConnect, ERR_OK);
    } else {
        ASSERT_NE(resConnect, ERR_OK);
    }
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: ScreenSessionConnectExtension02
 * @tc.desc: ScreenSessionConnectExtension02 func
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, ScreenSessionConnectExtension02, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    AAFwk::Want want;
    const std::string bundleName = "com.ohos.sceneboard";
    const std::string abilityName = "com.ohos.sceneboard.systemdialog";
    want.SetElementName(bundleName, abilityName);
    std::string paramKey = "requestReason";
    std::string paramValue = "onPlugIn";
    want.SetParam(paramKey, paramValue);
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto resConnect = AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(
        want, abilityConnectionStub, nullptr, DEFAULT_VALUE);
    IPCSkeleton::SetCallingIdentity(identity);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(resConnect, ERR_OK);
    } else {
        ASSERT_NE(resConnect, ERR_OK);
    }
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: ScreenSessionDisconnectExtension
 * @tc.desc: ScreenSessionDisconnectExtension func test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, ScreenSessionDisconnectExtension, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    AAFwk::Want want;
    want.SetElementName("com.ohos.sceneboard",
        "com.ohos.sceneboard.systemdialog");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto resConnect = AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(
        want, abilityConnectionStub, nullptr, DEFAULT_VALUE);
    IPCSkeleton::SetCallingIdentity(identity);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(resConnect, ERR_OK);
        {
            std::unique_lock<std::mutex> lock(connectedMutex_);
            static_cast<void>(connectedCv_.wait_for(lock, std::chrono::milliseconds(EXTENSION_CONNECT_OUT_TIME)));
        }
        auto resDisconnect = AAFwk::ExtensionManagerClient::GetInstance().DisconnectAbility(abilityConnectionStub);
        ASSERT_EQ(resDisconnect, NO_ERROR);
    } else {
        ASSERT_NE(resConnect, ERR_OK);
    }
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: SendMessage
 * @tc.desc: SendMessage func test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, SendMessage, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    AAFwk::Want want;
    want.SetElementName("com.ohos.sceneboard",
        "com.ohos.sceneboard.systemdialog");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto resConnect = AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(
        want, abilityConnectionStub, nullptr, DEFAULT_VALUE);
    IPCSkeleton::SetCallingIdentity(identity);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(resConnect, ERR_OK);
        {
            std::unique_lock<std::mutex> lock(connectedMutex_);
            static_cast<void>(connectedCv_.wait_for(lock, std::chrono::milliseconds(EXTENSION_CONNECT_OUT_TIME)));
        }
        MessageParcel data;
        MessageParcel reply;
        data.WriteString16(Str8ToStr16("SA"));
        data.WriteString16(Str8ToStr16("ScreenSessionManager"));
        abilityConnectionStub->SendMessageSync(TRANS_CMD_SEND_SNAPSHOT_RECT, data, reply);
        EXPECT_EQ(reply.ReadInt32(), 0);
        EXPECT_EQ(reply.ReadInt32(), 0);
        EXPECT_EQ(reply.ReadInt32(), 0);
        EXPECT_EQ(reply.ReadInt32(), 0);
        EXPECT_EQ(reply.ReadInt32(), 0);
        auto resDisconnect = AAFwk::ExtensionManagerClient::GetInstance().DisconnectAbility(abilityConnectionStub);
        ASSERT_EQ(resDisconnect, NO_ERROR);
    } else {
        ASSERT_NE(resConnect, ERR_OK);
    }
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: SendMessageBlock
 * @tc.desc: SendMessageBlock func test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, SendMessageBlock, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    AAFwk::Want want;
    want.SetElementName("com.ohos.sceneboard",
        "com.ohos.sceneboard.systemdialog");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto resConnect = AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(
        want, abilityConnectionStub, nullptr, DEFAULT_VALUE);
    IPCSkeleton::SetCallingIdentity(identity);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(resConnect, ERR_OK);
        {
            std::unique_lock<std::mutex> lock(connectedMutex_);
            static_cast<void>(connectedCv_.wait_for(lock, std::chrono::milliseconds(EXTENSION_CONNECT_OUT_TIME)));
        }
        MessageParcel data;
        MessageParcel reply;
        data.WriteString16(Str8ToStr16("SA"));
        data.WriteString16(Str8ToStr16("ScreenSessionManager"));
        abilityConnectionStub->SendMessageSyncBlock(TRANS_CMD_SEND_SNAPSHOT_RECT, data, reply);
        EXPECT_EQ(reply.ReadInt32(), 0);
        EXPECT_EQ(reply.ReadInt32(), 0);
        EXPECT_EQ(reply.ReadInt32(), 0);
        EXPECT_EQ(reply.ReadInt32(), 0);
        EXPECT_EQ(reply.ReadInt32(), 0);
        auto resDisconnect = AAFwk::ExtensionManagerClient::GetInstance().DisconnectAbility(abilityConnectionStub);
        ASSERT_EQ(resDisconnect, NO_ERROR);
    } else {
        ASSERT_NE(resConnect, ERR_OK);
    }
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: OnRemoteRequest
 * @tc.desc: OnRemoteRequest func test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, OnRemoteRequest, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    AAFwk::Want want;
    want.SetElementName("com.ohos.sceneboard",
        "com.ohos.sceneboard.systemdialog");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    auto resConnect = AAFwk::ExtensionManagerClient::GetInstance().ConnectServiceExtensionAbility(
        want, abilityConnectionStub, nullptr, DEFAULT_VALUE);
    IPCSkeleton::SetCallingIdentity(identity);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(resConnect, ERR_OK);
        {
            std::unique_lock<std::mutex> lock(connectedMutex_);
            static_cast<void>(connectedCv_.wait_for(lock, std::chrono::milliseconds(EXTENSION_CONNECT_OUT_TIME)));
        }
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        data.WriteString16(Str8ToStr16("SA"));
        data.WriteString16(Str8ToStr16("ScreenSessionManager"));
        abilityConnectionStub->OnRemoteRequest(TRANS_CMD_SEND_SNAPSHOT_RECT, data, reply, option);
        EXPECT_EQ(reply.ReadInt32(), 0);
        EXPECT_EQ(reply.ReadInt32(), 0);
        EXPECT_EQ(reply.ReadInt32(), 0);
        EXPECT_EQ(reply.ReadInt32(), 0);
        EXPECT_EQ(reply.ReadInt32(), 0);
        auto resDisconnect = AAFwk::ExtensionManagerClient::GetInstance().DisconnectAbility(abilityConnectionStub);
        ASSERT_EQ(resDisconnect, NO_ERROR);
    } else {
        ASSERT_NE(resConnect, ERR_OK);
    }
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: GetScreenId
 * @tc.desc: GetScreenId func test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, GetScreenId, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    int32_t expection = 0;
    EXPECT_EQ(abilityConnectionStub->GetScreenId(), expection);
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: GetLeft
 * @tc.desc: GetLeft func test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, GetLeft, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    int32_t expection = 0;
    EXPECT_EQ(abilityConnectionStub->GetLeft(), expection);
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: GetTop
 * @tc.desc: GetTop func test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, GetTop, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    int32_t expection = 0;
    EXPECT_EQ(abilityConnectionStub->GetTop(), expection);
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: GetWidth
 * @tc.desc: GetWidth func test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, GetWidth, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    int32_t expection = 0;
    EXPECT_EQ(abilityConnectionStub->GetWidth(), expection);
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: GetHeight
 * @tc.desc: GetHeight func test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, GetHeight, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    int32_t expection = 0;
    EXPECT_EQ(abilityConnectionStub->GetHeight(), expection);
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: GetErrCode
 * @tc.desc: GetErrCode func test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, GetErrCode, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    uint32_t expection = 0;
    EXPECT_EQ(abilityConnectionStub->GetErrCode(), expection);
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}

/**
 * @tc.name: EraseErrCode
 * @tc.desc: EraseErrCode func test
 * @tc.type: FUNC
 */
HWTEST_F(ScreenSessionAbilityConnectionUtTest, EraseErrCode, Function | SmallTest | Level1)
{
    sptr<ScreenSessionAbilityConnectionStub> abilityConnectionStub(
        new (std::nothrow) ScreenSessionAbilityConnectionStub());
    ASSERT_NE(abilityConnectionStub, nullptr);
    abilityConnectionStub->errCode_ = 1;
    uint32_t expection = 0;
    abilityConnectionStub->EraseErrCode();
    EXPECT_EQ(abilityConnectionStub->errCode_, expection);
    abilityConnectionStub.clear();
    abilityConnectionStub = nullptr;
}
}
} // namespace Rosen
} // namespace OHOS