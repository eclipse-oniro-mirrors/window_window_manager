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
#include "session_manager/include/extension_session_manager.h"
#include "session/host/include/extension_session.h"
#include "window_manager_hilog.h"
#include "iremote_object_mocker.h"
#include "mock_ability_manager_client.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class ExtensionSessionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

void ExtensionSessionManagerTest::SetUpTestCase() {}

void ExtensionSessionManagerTest::TearDownTestCase() {}

void ExtensionSessionManagerTest::SetUp() {}

void ExtensionSessionManagerTest::TearDown() {}

namespace {
/**
 * @tc.name: RequestExtensionSession
 * @tc.desc: RequestExtensionSession Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, RequestExtensionSession, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "RequestExtensionSession";
    ASSERT_NE(nullptr, ExtensionSessionManager::GetInstance().RequestExtensionSession(sessionInfo));
    // Atomic service branch
    AAFwk::Want want;
    sessionInfo.want = std::make_shared<AAFwk::Want>(want);
    ASSERT_NE(nullptr, sessionInfo.want);
    ASSERT_NE(nullptr, ExtensionSessionManager::GetInstance().RequestExtensionSession(sessionInfo));
    sessionInfo.want->SetParam(AAFwk::SCREEN_MODE_KEY, 1);
    ASSERT_NE(nullptr, ExtensionSessionManager::GetInstance().RequestExtensionSession(sessionInfo));
    sessionInfo.want->SetParam("ohos.extra.param.key.setUIExtensionTransparent", true);
    ASSERT_NE(nullptr, ExtensionSessionManager::GetInstance().RequestExtensionSession(sessionInfo));
}

/**
 * @tc.name: RequestExtensionSessionActivation01
 * @tc.desc: RequestExtensionSessionActivation Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, RequestExtensionSessionActivation01, TestSize.Level1)
{
    SessionInfo info;
    sptr<ExtensionSession> extensionSession = sptr<ExtensionSession>::MakeSptr(info);
    ASSERT_EQ(WSError::WS_OK,
              ExtensionSessionManager::GetInstance().RequestExtensionSessionBackground(extensionSession, nullptr));
}

void activationFunc(WSError we, int32_t errorCode) {}

void func(WSError we) {}

/**
 * @tc.name: RequestExtensionSessionActivation02
 * @tc.desc: RequestExtensionSessionActivation Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, RequestExtensionSessionActivation02, TestSize.Level1)
{
    SessionInfo info;
    sptr<ExtensionSession> extensionSession = sptr<ExtensionSession>::MakeSptr(info);
    ExtensionSessionManager* instance = &ExtensionSessionManager::GetInstance();
    ASSERT_EQ(WSError::WS_OK, instance->RequestExtensionSessionActivation(extensionSession, 1, nullptr));
    usleep(WAIT_SYNC_IN_NS);

    ASSERT_EQ(WSError::WS_OK, instance->RequestExtensionSessionActivation(extensionSession, 1, (activationFunc)));
    usleep(WAIT_SYNC_IN_NS);

    extensionSession->persistentId_ = -1;
    ASSERT_EQ(WSError::WS_OK, instance->RequestExtensionSessionActivation(extensionSession, 1, nullptr));
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: RequestExtensionSessionBackground01
 * @tc.desc: RequestExtensionSessionBackground Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, RequestExtensionSessionBackground01, TestSize.Level1)
{
    SessionInfo info;
    sptr<ExtensionSession> extensionSession = sptr<ExtensionSession>::MakeSptr(info);
    ASSERT_EQ(WSError::WS_OK,
              ExtensionSessionManager::GetInstance().RequestExtensionSessionBackground(extensionSession, nullptr));
}

/**
 * @tc.name: RequestExtensionSessionBackground02
 * @tc.desc: RequestExtensionSessionBackground Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, RequestExtensionSessionBackground02, TestSize.Level1)
{
    SessionInfo info;
    sptr<ExtensionSession> extensionSession = sptr<ExtensionSession>::MakeSptr(info);
    ExtensionSessionManager* instance = &ExtensionSessionManager::GetInstance();
    ASSERT_EQ(WSError::WS_OK, instance->RequestExtensionSessionBackground(extensionSession, nullptr));
    usleep(WAIT_SYNC_IN_NS);

    ASSERT_EQ(WSError::WS_OK, instance->RequestExtensionSessionBackground(extensionSession, func));
    usleep(WAIT_SYNC_IN_NS);

    extensionSession->persistentId_ = -1;
    ASSERT_EQ(WSError::WS_OK, instance->RequestExtensionSessionBackground(extensionSession, nullptr));
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: RequestExtensionSessionDestruction01
 * @tc.desc: RequestExtensionSessionDestruction Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, RequestExtensionSessionDestruction01, TestSize.Level1)
{
    SessionInfo info;
    sptr<ExtensionSession> extensionSession = sptr<ExtensionSession>::MakeSptr(info);
    ASSERT_EQ(WSError::WS_OK,
              ExtensionSessionManager::GetInstance().RequestExtensionSessionDestruction(extensionSession, nullptr));
}

/**
 * @tc.name: RequestExtensionSessionDestructionDone01
 * @tc.desc: RequestExtensionSessionDestructionDone Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, RequestExtensionSessionDestructionDone01, TestSize.Level1)
{
    SessionInfo info;
    sptr<ExtensionSession> extensionSession = new ExtensionSession(info);
    ASSERT_EQ(WSError::WS_OK,
              ExtensionSessionManager::GetInstance().RequestExtensionSessionDestructionDone(extensionSession));
}

/**
 * @tc.name: RequestExtensionSessionDestruction03
 * @tc.desc: RequestExtensionSessionDestruction Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, RequestExtensionSessionDestruction03, TestSize.Level1)
{
    SessionInfo info;
    sptr<ExtensionSession> extensionSession = sptr<ExtensionSession>::MakeSptr(info);
    ExtensionSessionManager* instance = &ExtensionSessionManager::GetInstance();
    ASSERT_EQ(WSError::WS_OK, instance->RequestExtensionSessionDestruction(extensionSession, nullptr));
    usleep(WAIT_SYNC_IN_NS);

    ASSERT_EQ(WSError::WS_OK, instance->RequestExtensionSessionDestruction(extensionSession, func));
    usleep(WAIT_SYNC_IN_NS);

    extensionSession->persistentId_ = -1;
    ASSERT_EQ(WSError::WS_OK, instance->RequestExtensionSessionDestruction(extensionSession, nullptr));
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: RequestExtensionSessionDestructionDone03
 * @tc.desc: RequestExtensionSessionDestructionDone Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, RequestExtensionSessionDestructionDone03, TestSize.Level1)
{
    SessionInfo info;
    sptr<ExtensionSession> extensionSession = new ExtensionSession(info);
    ExtensionSessionManager* instance = &ExtensionSessionManager::GetInstance();
    ASSERT_EQ(WSError::WS_OK, instance->RequestExtensionSessionDestructionDone(extensionSession));
    usleep(WAIT_SYNC_IN_NS);

    extensionSession->persistentId_ = -1;
    ASSERT_EQ(WSError::WS_OK, instance->RequestExtensionSessionDestructionDone(extensionSession));
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: GetInstance
 * @tc.desc: ExtensionSessionManager get instance and init
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, GetInstance, TestSize.Level1)
{
    ExtensionSessionManager* instance = &ExtensionSessionManager::GetInstance();
    ASSERT_NE(nullptr, instance);
}

/**
 * @tc.name: SetAbilitySessionInfo
 * @tc.desc: ExtensionSessionManager set ability session info
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, SetAbilitySessionInfo, TestSize.Level1)
{
    AAFwk::Want want;
    SessionInfo infoInput;
    infoInput.want = std::make_shared<AAFwk::Want>(want);
    sptr<ExtensionSession> extSession = new ExtensionSession(infoInput);
    ExtensionSessionManager* instance = &ExtensionSessionManager::GetInstance();
    sptr<AAFwk::SessionInfo> result = instance->SetAbilitySessionInfo(extSession);
    int32_t persistentId = extSession->GetPersistentId();
    ASSERT_EQ(result->persistentId, persistentId);

    result = instance->SetAbilitySessionInfo(extSession);
    extSession->sessionInfo_.want = nullptr;
    ASSERT_EQ(result->persistentId, persistentId);
}

/**
 * @tc.name: RequestExtensionSessionDestruction02
 * @tc.desc: RequestExtensionSessionDestruction Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, RequestExtensionSessionDestruction02, TestSize.Level1)
{
    AAFwk::Want want;
    SessionInfo infoInput;
    infoInput.want = std::make_shared<AAFwk::Want>(want);
    sptr<ExtensionSession> extSession = nullptr;
    ExtensionSessionManager* instance = &ExtensionSessionManager::GetInstance();
    WSError result01 = instance->RequestExtensionSessionDestruction(extSession, nullptr);
    EXPECT_EQ(result01, WSError::WS_OK);
}

/**
 * @tc.name: RequestExtensionSessionDestructionDone02
 * @tc.desc: RequestExtensionSessionDestructionDone Test
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, RequestExtensionSessionDestructionDone02, TestSize.Level1)
{
    AAFwk::Want want;
    SessionInfo infoInput;
    infoInput.want = std::make_shared<AAFwk::Want>(want);
    sptr<ExtensionSession> extSession = nullptr;
    ExtensionSessionManager* instance = &ExtensionSessionManager::GetInstance();
    WSError result01 = instance->RequestExtensionSessionDestructionDone(extSession);
    EXPECT_EQ(result01, WSError::WS_OK);
}

/**
 * @tc.name: UpdateAbilitySessionInfo02
 * @tc.desc: UpdateAbilitySessionInfo maps all host related fields to ability session info
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, UpdateAbilitySessionInfo02, TestSize.Level1)
{
    AAFwk::Want want;
    want.SetParam("ut_want_key", 42);
    SessionInfo sessionInfo;
    sessionInfo.want = std::make_shared<AAFwk::Want>(want);
    ASSERT_NE(nullptr, sessionInfo.want);
    sessionInfo.callerToken_ = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(nullptr, sessionInfo.callerToken_);
    sessionInfo.rootToken_ = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(nullptr, sessionInfo.rootToken_);
    sessionInfo.realParentId_ = 101;
    sessionInfo.isAsyncModalBinding_ = true;
    sessionInfo.uiExtensionUsage_ = 2;
    sessionInfo.parentWindowType_ = 8;
    sessionInfo.brandColorStr_ = "#FF0000";
    sessionInfo.callerPersistentId_ = 102;
    sessionInfo.config_.displayId_ = 3;
    sessionInfo.config_.density_ = 2.5f;
    sessionInfo.config_.orientation_ = 4;
    sessionInfo.config_.dpiFollowStrategy_ = DpiFollowStrategy::FOLLOW_HOST_DPI_UEA;

    sptr<AAFwk::SessionInfo> abilitySessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    ASSERT_NE(nullptr, abilitySessionInfo);
    abilitySessionInfo->persistentId = 999; // owned by SetAbilitySessionInfo, not updated here
    abilitySessionInfo->sessionToken = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<IRemoteObject> sessionToken = abilitySessionInfo->sessionToken;

    ExtensionSessionManager::UpdateAbilitySessionInfo(abilitySessionInfo, sessionInfo);

    EXPECT_EQ(sessionInfo.callerToken_.GetRefPtr(), abilitySessionInfo->callerToken.GetRefPtr());
    EXPECT_EQ(sessionInfo.rootToken_.GetRefPtr(), abilitySessionInfo->parentToken.GetRefPtr());
    EXPECT_EQ(sessionInfo.realParentId_, abilitySessionInfo->realHostWindowId);
    EXPECT_TRUE(abilitySessionInfo->isAsyncModalBinding);
    EXPECT_EQ(sessionInfo.uiExtensionUsage_, static_cast<uint32_t>(abilitySessionInfo->uiExtensionUsage));
    EXPECT_EQ(sessionInfo.parentWindowType_, abilitySessionInfo->parentWindowType);
    EXPECT_EQ(sessionInfo.brandColorStr_, abilitySessionInfo->brandColorStr);
    EXPECT_EQ(sessionInfo.callerPersistentId_, abilitySessionInfo->callerPersistentId);
    EXPECT_EQ(sessionInfo.config_.displayId_, abilitySessionInfo->displayId);
    EXPECT_FLOAT_EQ(sessionInfo.config_.density_, abilitySessionInfo->density);
    EXPECT_EQ(sessionInfo.config_.orientation_, abilitySessionInfo->orientation);
    EXPECT_EQ(static_cast<uint32_t>(sessionInfo.config_.dpiFollowStrategy_),
        abilitySessionInfo->dpiFollowStrategy);
    EXPECT_EQ(42, abilitySessionInfo->want.GetIntParam("ut_want_key", 0));
    EXPECT_EQ(999, abilitySessionInfo->persistentId);
    EXPECT_EQ(sessionToken.GetRefPtr(), abilitySessionInfo->sessionToken.GetRefPtr());
}

/**
 * @tc.name: UpdateAbilitySessionInfo03
 * @tc.desc: UpdateAbilitySessionInfo keeps original want when session info has no want
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, UpdateAbilitySessionInfo03, TestSize.Level1)
{
    sptr<AAFwk::SessionInfo> abilitySessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    ASSERT_NE(nullptr, abilitySessionInfo);
    AAFwk::Want marker;
    marker.SetParam("ut_marker", 7);
    abilitySessionInfo->want = marker;

    SessionInfo sessionInfo; // want is nullptr
    sessionInfo.realParentId_ = 103;
    ExtensionSessionManager::UpdateAbilitySessionInfo(abilitySessionInfo, sessionInfo);
    EXPECT_EQ(7, abilitySessionInfo->want.GetIntParam("ut_marker", 0));
    EXPECT_EQ(sessionInfo.realParentId_, abilitySessionInfo->realHostWindowId);
}

/**
 * @tc.name: UpdateSessionInfo01
 * @tc.desc: UpdateSessionInfo copies all host related fields
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, UpdateSessionInfo01, TestSize.Level1)
{
    SessionInfo newSessionInfo;
    newSessionInfo.callerToken_ = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(nullptr, newSessionInfo.callerToken_);
    newSessionInfo.rootToken_ = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(nullptr, newSessionInfo.rootToken_);
    newSessionInfo.realParentId_ = 201;
    newSessionInfo.parentWindowType_ = 8;
    newSessionInfo.brandColorStr_ = "#00FF00";
    newSessionInfo.callerPersistentId_ = 202;
    newSessionInfo.config_.displayId_ = 5;
    newSessionInfo.config_.density_ = 3.5f;
    newSessionInfo.config_.orientation_ = 6;

    SessionInfo editSessionInfo;
    ExtensionSessionManager::UpdateSessionInfo(editSessionInfo, newSessionInfo);

    EXPECT_EQ(newSessionInfo.callerToken_.GetRefPtr(), editSessionInfo.callerToken_.GetRefPtr());
    EXPECT_EQ(newSessionInfo.rootToken_.GetRefPtr(), editSessionInfo.rootToken_.GetRefPtr());
    EXPECT_EQ(newSessionInfo.realParentId_, editSessionInfo.realParentId_);
    EXPECT_EQ(newSessionInfo.parentWindowType_, editSessionInfo.parentWindowType_);
    EXPECT_EQ(newSessionInfo.brandColorStr_, editSessionInfo.brandColorStr_);
    EXPECT_EQ(newSessionInfo.callerPersistentId_, editSessionInfo.callerPersistentId_);
    EXPECT_EQ(newSessionInfo.config_.displayId_, editSessionInfo.config_.displayId_);
    EXPECT_FLOAT_EQ(newSessionInfo.config_.density_, editSessionInfo.config_.density_);
    EXPECT_EQ(newSessionInfo.config_.orientation_, editSessionInfo.config_.orientation_);
}

/**
 * @tc.name: UpdateSessionInfo02
 * @tc.desc: UpdateSessionInfo overwrites old host values with new host values
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, UpdateSessionInfo02, TestSize.Level1)
{
    SessionInfo editSessionInfo;
    editSessionInfo.realParentId_ = 301;
    editSessionInfo.parentWindowType_ = 1;
    editSessionInfo.brandColorStr_ = "#OLD";
    editSessionInfo.callerPersistentId_ = 302;
    editSessionInfo.config_.displayId_ = 1;
    editSessionInfo.config_.density_ = 1.0f;
    editSessionInfo.config_.orientation_ = 0;

    SessionInfo newSessionInfo;
    newSessionInfo.realParentId_ = 303;
    newSessionInfo.parentWindowType_ = 8;
    newSessionInfo.brandColorStr_ = "#NEW";
    newSessionInfo.callerPersistentId_ = 304;
    newSessionInfo.config_.displayId_ = 9;
    newSessionInfo.config_.density_ = 4.5f;
    newSessionInfo.config_.orientation_ = 7;

    ExtensionSessionManager::UpdateSessionInfo(editSessionInfo, newSessionInfo);

    EXPECT_EQ(newSessionInfo.realParentId_, editSessionInfo.realParentId_);
    EXPECT_EQ(newSessionInfo.parentWindowType_, editSessionInfo.parentWindowType_);
    EXPECT_EQ(newSessionInfo.brandColorStr_, editSessionInfo.brandColorStr_);
    EXPECT_EQ(newSessionInfo.callerPersistentId_, editSessionInfo.callerPersistentId_);
    EXPECT_EQ(newSessionInfo.config_.displayId_, editSessionInfo.config_.displayId_);
    EXPECT_FLOAT_EQ(newSessionInfo.config_.density_, editSessionInfo.config_.density_);
    EXPECT_EQ(newSessionInfo.config_.orientation_, editSessionInfo.config_.orientation_);
}

/**
 * @tc.name: UpdateSessionInfo03
 * @tc.desc: UpdateSessionInfo keeps fields out of the migration list unchanged
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, UpdateSessionInfo03, TestSize.Level1)
{
    SessionInfo editSessionInfo;
    editSessionInfo.bundleName_ = "oldBundle";
    editSessionInfo.abilityName_ = "oldAbility";
    editSessionInfo.persistentId_ = 401;
    editSessionInfo.uiExtensionUsage_ = 1;
    editSessionInfo.isAsyncModalBinding_ = true;
    editSessionInfo.config_.dpiFollowStrategy_ = DpiFollowStrategy::FOLLOW_HOST_DPI_UEA;

    SessionInfo newSessionInfo;
    newSessionInfo.bundleName_ = "newBundle";
    newSessionInfo.abilityName_ = "newAbility";
    newSessionInfo.persistentId_ = 402;
    newSessionInfo.uiExtensionUsage_ = 3;
    newSessionInfo.isAsyncModalBinding_ = false;
    newSessionInfo.config_.dpiFollowStrategy_ = DpiFollowStrategy::FOLLOW_HOST_DPI_ALL;
    newSessionInfo.realParentId_ = 403;

    ExtensionSessionManager::UpdateSessionInfo(editSessionInfo, newSessionInfo);

    EXPECT_EQ(std::string("oldBundle"), editSessionInfo.bundleName_);
    EXPECT_EQ(std::string("oldAbility"), editSessionInfo.abilityName_);
    EXPECT_EQ(401, editSessionInfo.persistentId_);
    EXPECT_EQ(1u, editSessionInfo.uiExtensionUsage_);
    EXPECT_TRUE(editSessionInfo.isAsyncModalBinding_);
    EXPECT_TRUE(DpiFollowStrategy::FOLLOW_HOST_DPI_UEA == editSessionInfo.config_.dpiFollowStrategy_);
    EXPECT_EQ(newSessionInfo.realParentId_, editSessionInfo.realParentId_);
}

/**
 * @tc.name: UpdateExtensionSessionParent01
 * @tc.desc: UpdateExtensionSessionParent with nullptr session
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, UpdateExtensionSessionParent01, TestSize.Level1)
{
    AAFwk::MockAbilityManagerClient::ClearAll();
    SessionInfo sessionInfo;
    ASSERT_EQ(WSError::WS_OK,
        ExtensionSessionManager::GetInstance().UpdateExtensionSessionParent(nullptr, 1, sessionInfo));
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(nullptr, AAFwk::MockAbilityManagerClient::GetLastStartUIExtensionAbilityInfo().GetRefPtr());
}

/**
 * @tc.name: UpdateExtensionSessionParent02
 * @tc.desc: UpdateExtensionSessionParent updates AMS payload and local session info on success
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, UpdateExtensionSessionParent02, TestSize.Level1)
{
    AAFwk::MockAbilityManagerClient::ClearAll();
    AAFwk::MockAbilityManagerClient::SetStartUIExtensionAbilityResult(ERR_OK);

    SessionInfo oldSessionInfo;
    oldSessionInfo.abilityName_ = "UpdateExtensionSessionParent02";
    oldSessionInfo.realParentId_ = 501;
    oldSessionInfo.parentWindowType_ = 1;
    oldSessionInfo.brandColorStr_ = "#OLD";
    oldSessionInfo.callerPersistentId_ = 502;
    oldSessionInfo.config_.displayId_ = 1;
    oldSessionInfo.config_.density_ = 1.0f;
    oldSessionInfo.config_.orientation_ = 0;
    sptr<ExtensionSession> extSession = sptr<ExtensionSession>::MakeSptr(oldSessionInfo);
    ASSERT_NE(nullptr, extSession);

    AAFwk::Want want;
    want.SetParam("ut_parent_key", 42);
    SessionInfo newSessionInfo;
    newSessionInfo.callerToken_ = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(nullptr, newSessionInfo.callerToken_);
    newSessionInfo.rootToken_ = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(nullptr, newSessionInfo.rootToken_);
    newSessionInfo.want = std::make_shared<AAFwk::Want>(want);
    ASSERT_NE(nullptr, newSessionInfo.want);
    newSessionInfo.realParentId_ = 503;
    newSessionInfo.parentWindowType_ = 8;
    newSessionInfo.brandColorStr_ = "#NEW";
    newSessionInfo.callerPersistentId_ = 504;
    newSessionInfo.config_.displayId_ = 6;
    newSessionInfo.config_.density_ = 2.5f;
    newSessionInfo.config_.orientation_ = 3;
    uint32_t newHostWindowId = 999;

    ASSERT_EQ(WSError::WS_OK, ExtensionSessionManager::GetInstance().UpdateExtensionSessionParent(
        extSession, newHostWindowId, newSessionInfo));
    usleep(WAIT_SYNC_IN_NS);

    // Verify the payload sent to AMS
    sptr<AAFwk::SessionInfo> payload = AAFwk::MockAbilityManagerClient::GetLastStartUIExtensionAbilityInfo();
    ASSERT_NE(nullptr, payload);
    EXPECT_EQ(newHostWindowId, payload->hostWindowId);
    EXPECT_EQ(extSession->GetPersistentId(), payload->persistentId);
    EXPECT_NE(nullptr, payload->sessionToken);
    EXPECT_EQ(newSessionInfo.callerToken_.GetRefPtr(), payload->callerToken.GetRefPtr());
    EXPECT_EQ(newSessionInfo.rootToken_.GetRefPtr(), payload->parentToken.GetRefPtr());
    EXPECT_EQ(newSessionInfo.realParentId_, payload->realHostWindowId);
    EXPECT_EQ(newSessionInfo.parentWindowType_, payload->parentWindowType);
    EXPECT_EQ(newSessionInfo.brandColorStr_, payload->brandColorStr);
    EXPECT_EQ(newSessionInfo.callerPersistentId_, payload->callerPersistentId);
    EXPECT_EQ(newSessionInfo.config_.displayId_, payload->displayId);
    EXPECT_FLOAT_EQ(newSessionInfo.config_.density_, payload->density);
    EXPECT_EQ(newSessionInfo.config_.orientation_, payload->orientation);
    EXPECT_EQ(42, payload->want.GetIntParam("ut_parent_key", 0));

    // Verify local session info is updated after IPC success
    const auto& updatedInfo = extSession->GetSessionInfo();
    EXPECT_EQ(newSessionInfo.callerToken_.GetRefPtr(), updatedInfo.callerToken_.GetRefPtr());
    EXPECT_EQ(newSessionInfo.rootToken_.GetRefPtr(), updatedInfo.rootToken_.GetRefPtr());
    EXPECT_EQ(newSessionInfo.realParentId_, updatedInfo.realParentId_);
    EXPECT_EQ(newSessionInfo.parentWindowType_, updatedInfo.parentWindowType_);
    EXPECT_EQ(newSessionInfo.brandColorStr_, updatedInfo.brandColorStr_);
    EXPECT_EQ(newSessionInfo.callerPersistentId_, updatedInfo.callerPersistentId_);
    EXPECT_EQ(newSessionInfo.config_.displayId_, updatedInfo.config_.displayId_);
    EXPECT_FLOAT_EQ(newSessionInfo.config_.density_, updatedInfo.config_.density_);
    EXPECT_EQ(newSessionInfo.config_.orientation_, updatedInfo.config_.orientation_);
}

/**
 * @tc.name: UpdateExtensionSessionParent03
 * @tc.desc: UpdateExtensionSessionParent keeps local session info unchanged on IPC failure
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, UpdateExtensionSessionParent03, TestSize.Level1)
{
    AAFwk::MockAbilityManagerClient::ClearAll();
    AAFwk::MockAbilityManagerClient::SetStartUIExtensionAbilityResult(AAFwk::INNER_ERR);

    SessionInfo oldSessionInfo;
    oldSessionInfo.abilityName_ = "UpdateExtensionSessionParent03";
    oldSessionInfo.realParentId_ = 601;
    oldSessionInfo.brandColorStr_ = "#OLD";
    oldSessionInfo.callerPersistentId_ = 602;
    oldSessionInfo.config_.displayId_ = 1;
    oldSessionInfo.config_.density_ = 1.0f;
    oldSessionInfo.config_.orientation_ = 0;
    sptr<ExtensionSession> extSession = sptr<ExtensionSession>::MakeSptr(oldSessionInfo);
    ASSERT_NE(nullptr, extSession);

    SessionInfo newSessionInfo;
    newSessionInfo.realParentId_ = 603;
    newSessionInfo.brandColorStr_ = "#NEW";
    newSessionInfo.callerPersistentId_ = 604;
    newSessionInfo.config_.displayId_ = 8;
    newSessionInfo.config_.density_ = 3.5f;
    newSessionInfo.config_.orientation_ = 5;
    uint32_t newHostWindowId = 888;

    ASSERT_EQ(WSError::WS_OK, ExtensionSessionManager::GetInstance().UpdateExtensionSessionParent(
        extSession, newHostWindowId, newSessionInfo));
    usleep(WAIT_SYNC_IN_NS);

    // AMS is still notified with the new parent
    sptr<AAFwk::SessionInfo> payload = AAFwk::MockAbilityManagerClient::GetLastStartUIExtensionAbilityInfo();
    ASSERT_NE(nullptr, payload);
    EXPECT_EQ(newHostWindowId, payload->hostWindowId);
    EXPECT_EQ(newSessionInfo.realParentId_, payload->realHostWindowId);

    // Local session info is kept unchanged on IPC failure
    const auto& updatedInfo = extSession->GetSessionInfo();
    EXPECT_EQ(oldSessionInfo.realParentId_, updatedInfo.realParentId_);
    EXPECT_EQ(oldSessionInfo.brandColorStr_, updatedInfo.brandColorStr_);
    EXPECT_EQ(oldSessionInfo.callerPersistentId_, updatedInfo.callerPersistentId_);
    EXPECT_EQ(oldSessionInfo.config_.displayId_, updatedInfo.config_.displayId_);
    EXPECT_FLOAT_EQ(oldSessionInfo.config_.density_, updatedInfo.config_.density_);
    EXPECT_EQ(oldSessionInfo.config_.orientation_, updatedInfo.config_.orientation_);
}

/**
 * @tc.name: UpdateExtensionSessionParent04
 * @tc.desc: UpdateExtensionSessionParent with invalid persistentId skips AMS call
 * @tc.type: FUNC
 */
HWTEST_F(ExtensionSessionManagerTest, UpdateExtensionSessionParent04, TestSize.Level1)
{
    AAFwk::MockAbilityManagerClient::ClearAll();
    AAFwk::MockAbilityManagerClient::SetStartUIExtensionAbilityResult(ERR_OK);

    SessionInfo oldSessionInfo;
    oldSessionInfo.abilityName_ = "UpdateExtensionSessionParent04";
    oldSessionInfo.realParentId_ = 701;
    sptr<ExtensionSession> extSession = sptr<ExtensionSession>::MakeSptr(oldSessionInfo);
    ASSERT_NE(nullptr, extSession);
    extSession->persistentId_ = -1; // invalid

    SessionInfo newSessionInfo;
    newSessionInfo.realParentId_ = 702;
    ASSERT_EQ(WSError::WS_OK, ExtensionSessionManager::GetInstance().UpdateExtensionSessionParent(
        extSession, 777, newSessionInfo));
    usleep(WAIT_SYNC_IN_NS);

    EXPECT_EQ(nullptr, AAFwk::MockAbilityManagerClient::GetLastStartUIExtensionAbilityInfo().GetRefPtr());
    EXPECT_EQ(oldSessionInfo.realParentId_, extSession->GetSessionInfo().realParentId_);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
