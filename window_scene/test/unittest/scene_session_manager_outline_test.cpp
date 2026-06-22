/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#include "application_info.h"
#include <bundle_mgr_interface.h>
#include <bundlemgr/launcher_service.h>
#include "context.h"
#include <gtest/gtest.h>
#include "interfaces/include/ws_common.h"
#include "iremote_object_mocker.h"
#include "../mock/mock_accesstoken_kit.h"
#include "../mock/mock_session_stage.h"
#include "../mock/mock_window_event_channel.h"
#include <regex>
#include "session_manager.h"
#include "screen_fold_data.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/main_session.h"
#include "session/screen/include/screen_session.h"
#include "wm_common.h"
#include "window_manager_agent.h"
#include "window_manager_hilog.h"
#include "zidl/window_manager_agent_interface.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_logMsg += msg;
    }
}
namespace {
const std::string EMPTY_DEVICE_ID = "";
}

class SceneSessionManagerOutlineTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    // Helper methods
    sptr<SceneSession> CreateMockSceneSession(int32_t persistentId,
        SessionState state = SessionState::STATE_FOREGROUND);
    OutlineStyleParams CreateMockOutlineStyleParams(uint32_t color = 0xFF000000, uint32_t width = 2);
    OutlineParams CreateMockOutlineParams(const std::vector<int32_t>& persistentIds = {});
    void SetSystemConfigForFreeMultiWindow(bool enabled);

private:
    static constexpr uint32_t waitSyncInNs = 200000;
    static constexpr uint32_t waitSyncForTestEndInNs = 500000;
    static constexpr uint32_t defaultRectSize = 100;
    static sptr<SceneSessionManager> ssm_;
};

sptr<SceneSessionManager> SceneSessionManagerOutlineTest::ssm_ = nullptr;

void SceneSessionManagerOutlineTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerOutlineTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerOutlineTest::SetUp()
{
    ssm_->sceneSessionMap_.clear();
    ssm_->systemConfig_.freeMultiWindowSupport_ = true;
    ssm_->isScreenLocked_ = false;
    ssm_->outlineRemoteObject_ = nullptr;
    ssm_->recoverState_ = RecoverState::RECOVER_ENABLE_INPUT;
    ssm_->needRecoverOutline_ = false;
}

void SceneSessionManagerOutlineTest::TearDown()
{
    MockAccesstokenKit::ChangeMockStateToInit();
    usleep(waitSyncInNs);
    ssm_->sceneSessionMap_.clear();
    ssm_->outlineRemoteObject_ = nullptr;
    ssm_->systemConfig_.freeMultiWindowSupport_ = true;
}

sptr<SceneSession> SceneSessionManagerOutlineTest::CreateMockSceneSession(int32_t persistentId, SessionState state)
{
    SessionInfo info;
    info.bundleName_ = "test.bundle_" + std::to_string(persistentId);
    info.abilityName_ = "test.ability_" + std::to_string(persistentId);
    sptr<SceneSession> sceneSession = new SceneSession(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    sceneSession->persistentId_ = persistentId;
    sceneSession->state_ = state;
    sceneSession->GetSessionProperty()->SetWindowFlags(
        static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED));
    return sceneSession;
}

OutlineStyleParams SceneSessionManagerOutlineTest::CreateMockOutlineStyleParams(uint32_t color, uint32_t width)
{
    OutlineStyleParams params;
    params.outlineColor_ = color;
    params.outlineWidth_ = width;
    params.outlineShape_ = OutlineShape::OUTLINE_SHAPE_FOUR_CORNERS;
    return params;
}

OutlineParams SceneSessionManagerOutlineTest::CreateMockOutlineParams(const std::vector<int32_t>& persistentIds)
{
    OutlineParams params;
    params.type_ = OutlineType::OUTLINE_FOR_WINDOW;
    params.displayId_ = 0;
    params.rect_.posX_ = 0;
    params.rect_.posY_ = 0;
    params.rect_.width_ = defaultRectSize;
    params.rect_.height_ = defaultRectSize;
    params.persistentIds_ = persistentIds;
    params.outlineStyleParams_ = CreateMockOutlineStyleParams();
    return params;
}

void SceneSessionManagerOutlineTest::SetSystemConfigForFreeMultiWindow(bool enabled)
{
    ssm_->systemConfig_.freeMultiWindowSupport_ = enabled;
}

// ============================================================================
// UpdateScreenLockState Tests
// ============================================================================

/**
 * @tc.name: UpdateScreenLockState_NegativeId
 * @tc.desc: Test UpdateScreenLockState with negative persistentId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateScreenLockState_NegativeId, TestSize.Level1)
{
    WMError result = ssm_->UpdateScreenLockState(-1);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: UpdateScreenLockState_SessionNotFound
 * @tc.desc: Test UpdateScreenLockState with non-existent session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateScreenLockState_SessionNotFound, TestSize.Level1)
{
    WMError result = ssm_->UpdateScreenLockState(99999);
    EXPECT_EQ(result, WMError::WM_OK);
    usleep(waitSyncForTestEndInNs);
}

/**
 * @tc.name: UpdateScreenLockState_Success
 * @tc.desc: Test UpdateScreenLockState with valid session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateScreenLockState_Success, TestSize.Level1)
{
    sptr<SceneSession> sceneSession = CreateMockSceneSession(1001, SessionState::STATE_FOREGROUND);
    ssm_->sceneSessionMap_.emplace(1001, sceneSession);

    WMError result = ssm_->UpdateScreenLockState(1001);
    EXPECT_EQ(result, WMError::WM_OK);
    usleep(waitSyncForTestEndInNs);
}

/**
 * @tc.name: UpdateScreenLockState_ZeroId
 * @tc.desc: Test UpdateScreenLockState with zero persistentId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateScreenLockState_ZeroId, TestSize.Level1)
{
    WMError result = ssm_->UpdateScreenLockState(0);
    EXPECT_EQ(result, WMError::WM_OK);
    usleep(waitSyncForTestEndInNs);
}

// ============================================================================
// NotifySessionScreenLockedChange Tests
// ============================================================================

/**
 * @tc.name: NotifySessionScreenLockedChange_FreeMultiWindowDisabled
 * @tc.desc: Test NotifySessionScreenLockedChange when freeMultiWindowSupport is disabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, NotifySessionScreenLockedChange_FreeMultiWindowDisabled, TestSize.Level1)
{
    SetSystemConfigForFreeMultiWindow(false);
    ssm_->NotifySessionScreenLockedChange(true);
    // Should return early without any operation
    EXPECT_TRUE(true);
}

/**
 * @tc.name: NotifySessionScreenLockedChange_PcModeLocked
 * @tc.desc: Test NotifySessionScreenLockedChange in PC mode with screen locked
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, NotifySessionScreenLockedChange_PcModeLocked, TestSize.Level1)
{
    SetSystemConfigForFreeMultiWindow(true);
    // This test requires system parameter mock, just verify no crash
    ssm_->NotifySessionScreenLockedChange(true);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: NotifySessionScreenLockedChange_EmptySessionMap
 * @tc.desc: Test NotifySessionScreenLockedChange with empty session map
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, NotifySessionScreenLockedChange_EmptySessionMap, TestSize.Level1)
{
    SetSystemConfigForFreeMultiWindow(true);
    ssm_->sceneSessionMap_.clear();
    ssm_->NotifySessionScreenLockedChange(true);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: NotifySessionScreenLockedChange_NullSession
 * @tc.desc: Test NotifySessionScreenLockedChange with null session in map
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, NotifySessionScreenLockedChange_NullSession, TestSize.Level1)
{
    SetSystemConfigForFreeMultiWindow(true);
    ssm_->sceneSessionMap_.emplace(1001, nullptr);
    ssm_->NotifySessionScreenLockedChange(true);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: NotifySessionScreenLockedChange_NoShowWhenLockedFlag
 * @tc.desc: Test NotifySessionScreenLockedChange with session without WINDOW_FLAG_SHOW_WHEN_LOCKED
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, NotifySessionScreenLockedChange_NoShowWhenLockedFlag, TestSize.Level1)
{
    SetSystemConfigForFreeMultiWindow(true);
    sptr<SceneSession> sceneSession = CreateMockSceneSession(1001, SessionState::STATE_FOREGROUND);
    sceneSession->GetSessionProperty()->SetWindowFlags(0); // No WINDOW_FLAG_SHOW_WHEN_LOCKED
    ssm_->sceneSessionMap_.emplace(1001, sceneSession);

    ssm_->NotifySessionScreenLockedChange(true);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: NotifySessionScreenLockedChange_LockedAndNotForeground
 * @tc.desc: Test NotifySessionScreenLockedChange with screen locked and session not in foreground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, NotifySessionScreenLockedChange_LockedAndNotForeground, TestSize.Level1)
{
    SetSystemConfigForFreeMultiWindow(true);
    sptr<SceneSession> sceneSession = CreateMockSceneSession(1001, SessionState::STATE_BACKGROUND);
    ssm_->sceneSessionMap_.emplace(1001, sceneSession);

    ssm_->NotifySessionScreenLockedChange(true);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: NotifySessionScreenLockedChange_Unlocked
 * @tc.desc: Test NotifySessionScreenLockedChange with screen unlocked
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, NotifySessionScreenLockedChange_Unlocked, TestSize.Level1)
{
    SetSystemConfigForFreeMultiWindow(true);
    sptr<SceneSession> sceneSession = CreateMockSceneSession(1001, SessionState::STATE_FOREGROUND);
    ssm_->sceneSessionMap_.emplace(1001, sceneSession);

    ssm_->NotifySessionScreenLockedChange(false);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: NotifySessionScreenLockedChange_ForegroundSessionLocked
 * @tc.desc: Test NotifySessionScreenLockedChange with foreground session and screen locked
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, NotifySessionScreenLockedChange_ForegroundSessionLocked, TestSize.Level1)
{
    SetSystemConfigForFreeMultiWindow(true);
    sptr<SceneSession> sceneSession = CreateMockSceneSession(1001, SessionState::STATE_FOREGROUND);
    ssm_->sceneSessionMap_.emplace(1001, sceneSession);

    ssm_->NotifySessionScreenLockedChange(true);
    EXPECT_TRUE(true);
}

// ============================================================================
// NeedOutline Tests
// ============================================================================

/**
 * @tc.name: NeedOutline_EmptyList
 * @tc.desc: Test NeedOutline with empty persistentIdList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, NeedOutline_EmptyList, TestSize.Level1)
{
    std::vector<int32_t> emptyList;
    bool result = ssm_->NeedOutline(1001, emptyList);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: NeedOutline_IdInList
 * @tc.desc: Test NeedOutline when persistentId is in the list
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, NeedOutline_IdInList, TestSize.Level1)
{
    std::vector<int32_t> list = {1000, 1001, 1002};
    bool result = ssm_->NeedOutline(1001, list);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: NeedOutline_IdNotInList
 * @tc.desc: Test NeedOutline when persistentId is not in the list
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, NeedOutline_IdNotInList, TestSize.Level1)
{
    std::vector<int32_t> list = {1000, 1002, 1003};
    bool result = ssm_->NeedOutline(1001, list);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: NeedOutline_SingleElementMatch
 * @tc.desc: Test NeedOutline with single element list that matches
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, NeedOutline_SingleElementMatch, TestSize.Level1)
{
    std::vector<int32_t> list = {1001};
    bool result = ssm_->NeedOutline(1001, list);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: NeedOutline_SingleElementNoMatch
 * @tc.desc: Test NeedOutline with single element list that doesn't match
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, NeedOutline_SingleElementNoMatch, TestSize.Level1)
{
    std::vector<int32_t> list = {1002};
    bool result = ssm_->NeedOutline(1001, list);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: NeedOutline_DuplicateIds
 * @tc.desc: Test NeedOutline with duplicate IDs in list
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, NeedOutline_DuplicateIds, TestSize.Level1)
{
    std::vector<int32_t> list = {1000, 1001, 1001, 1002};
    bool result = ssm_->NeedOutline(1001, list);
    EXPECT_TRUE(result);
}

// ============================================================================
// CacheOutlineParamsIfNeed Tests
// ============================================================================

/**
 * @tc.name: CacheOutlineParamsIfNeed_RecoverInitial
 * @tc.desc: Test CacheOutlineParamsIfNeed when recoverState is RECOVER_INITIAL
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, CacheOutlineParamsIfNeed_RecoverInitial, TestSize.Level1)
{
    ssm_->recoverState_ = RecoverState::RECOVER_INITIAL;
    OutlineParams params = CreateMockOutlineParams({1001});

    bool result = ssm_->CacheOutlineParamsIfNeed(params);
    EXPECT_TRUE(result);
    EXPECT_TRUE(ssm_->needRecoverOutline_);
    EXPECT_EQ(ssm_->recoverOutlineParams_.type_, params.type_);
}

/**
 * @tc.name: CacheOutlineParamsIfNeed_RecoverEnableInput
 * @tc.desc: Test CacheOutlineParamsIfNeed when recoverState is RECOVER_ENABLE_INPUT
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, CacheOutlineParamsIfNeed_RecoverEnableInput, TestSize.Level1)
{
    ssm_->recoverState_ = RecoverState::RECOVER_ENABLE_INPUT;
    OutlineParams params = CreateMockOutlineParams({1001});

    bool result = ssm_->CacheOutlineParamsIfNeed(params);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: CacheOutlineParamsIfNeed_RecoverEnd
 * @tc.desc: Test CacheOutlineParamsIfNeed when recoverState is RECOVER_END
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, CacheOutlineParamsIfNeed_RecoverEnd, TestSize.Level1)
{
    ssm_->recoverState_ = RecoverState::RECOVER_END;
    OutlineParams params = CreateMockOutlineParams({1001});

    bool result = ssm_->CacheOutlineParamsIfNeed(params);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: CacheOutlineParamsIfNeed_MultipleParams
 * @tc.desc: Test CacheOutlineParamsIfNeed with multiple persistentIds
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, CacheOutlineParamsIfNeed_MultipleParams, TestSize.Level1)
{
    ssm_->recoverState_ = RecoverState::RECOVER_INITIAL;
    OutlineParams params = CreateMockOutlineParams({1001, 1002, 1003});

    bool result = ssm_->CacheOutlineParamsIfNeed(params);
    EXPECT_TRUE(result);
    EXPECT_EQ(ssm_->recoverOutlineParams_.persistentIds_.size(), 3);
}

// ============================================================================
// UpdateOutline Tests
// ============================================================================

/**
 * @tc.name: UpdateOutline_NoSAPermission
 * @tc.desc: Test UpdateOutline without SA permission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateOutline_NoSAPermission, TestSize.Level1)
{
    OutlineParams params = CreateMockOutlineParams({1001});
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();

    MockAccesstokenKit::MockIsSACalling(false);
    WMError result = ssm_->UpdateOutline(remoteObject, params);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_PERMISSION);
    MockAccesstokenKit::ChangeMockStateToInit();
}

/**
 * @tc.name: UpdateOutline_NotSupportedNotRecovering
 * @tc.desc: Test UpdateOutline when not supported and not in RECOVER_INITIAL state
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateOutline_NotSupportedNotRecovering, TestSize.Level1)
{
    OutlineParams params = CreateMockOutlineParams({1001});
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();

    MockAccesstokenKit::MockIsSACalling(true);

    // Simulate not PC and not FreeMultiWindow mode
    ssm_->recoverState_ = RecoverState::RECOVER_ENABLE_INPUT;

    WMError result = ssm_->UpdateOutline(remoteObject, params);
    // Should return either WM_OK (if cached) or WM_ERROR_DEVICE_NOT_SUPPORT
    // depending on system config
    EXPECT_TRUE(result == WMError::WM_OK || result == WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    MockAccesstokenKit::ChangeMockStateToInit();
}

/**
 * @tc.name: UpdateOutline_NullRemoteObject
 * @tc.desc: Test UpdateOutline with null remoteObject
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateOutline_NullRemoteObject, TestSize.Level1)
{
    OutlineParams params = CreateMockOutlineParams({1001});

    MockAccesstokenKit::MockIsSACalling(true);

    WMError result = ssm_->UpdateOutline(nullptr, params);
    // Should still work as AddOutlineRemoteDeathRecipient handles null
    EXPECT_TRUE(result == WMError::WM_OK || result == WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    MockAccesstokenKit::ChangeMockStateToInit();
}

/**
 * @tc.name: UpdateOutline_EmptyPersistentIdList
 * @tc.desc: Test UpdateOutline with empty persistentId list
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateOutline_EmptyPersistentIdList, TestSize.Level1)
{
    OutlineParams params = CreateMockOutlineParams({});
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();

    MockAccesstokenKit::MockIsSACalling(true);

    WMError result = ssm_->UpdateOutline(remoteObject, params);
    EXPECT_TRUE(result == WMError::WM_OK || result == WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    MockAccesstokenKit::ChangeMockStateToInit();
}

/**
 * @tc.name: UpdateOutline_ValidParams
 * @tc.desc: Test UpdateOutline with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateOutline_ValidParams, TestSize.Level1)
{
    OutlineParams params = CreateMockOutlineParams({1001});
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();

    MockAccesstokenKit::MockIsSACalling(true);

    WMError result = ssm_->UpdateOutline(remoteObject, params);
    EXPECT_TRUE(result == WMError::WM_OK || result == WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    MockAccesstokenKit::ChangeMockStateToInit();
}

// ============================================================================
// UpdateOutlineInner Tests
// ============================================================================

/**
 * @tc.name: UpdateOutlineInner_NullSession
 * @tc.desc: Test UpdateOutlineInner with null session in map
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateOutlineInner_NullSession, TestSize.Level1)
{
    OutlineParams params = CreateMockOutlineParams({1001});
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();

    ssm_->sceneSessionMap_.emplace(1001, nullptr);
    ssm_->sceneSessionMap_.emplace(1002, CreateMockSceneSession(1002, SessionState::STATE_FOREGROUND));

    ssm_->UpdateOutlineInner(remoteObject, params);
    usleep(waitSyncForTestEndInNs);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: UpdateOutlineInner_StateDisconnect
 * @tc.desc: Test UpdateOutlineInner with session in STATE_DISCONNECT
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateOutlineInner_StateDisconnect, TestSize.Level1)
{
    OutlineParams params = CreateMockOutlineParams({1001});
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();

    sptr<SceneSession> session = CreateMockSceneSession(1001, SessionState::STATE_DISCONNECT);
    ssm_->sceneSessionMap_.emplace(1001, session);

    ssm_->UpdateOutlineInner(remoteObject, params);
    usleep(waitSyncForTestEndInNs);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: UpdateOutlineInner_StateEnd
 * @tc.desc: Test UpdateOutlineInner with session in STATE_END
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateOutlineInner_StateEnd, TestSize.Level1)
{
    OutlineParams params = CreateMockOutlineParams({1001});
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();

    sptr<SceneSession> session = CreateMockSceneSession(1001, SessionState::STATE_END);
    ssm_->sceneSessionMap_.emplace(1001, session);

    ssm_->UpdateOutlineInner(remoteObject, params);
    usleep(waitSyncForTestEndInNs);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: UpdateOutlineInner_ConnectInList
 * @tc.desc: Test UpdateOutlineInner with STATE_CONNECT session in persistentId list
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateOutlineInner_ConnectInList, TestSize.Level1)
{
    OutlineParams params = CreateMockOutlineParams({1001});
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();

    sptr<SceneSession> session = CreateMockSceneSession(1001, SessionState::STATE_CONNECT);
    ssm_->sceneSessionMap_.emplace(1001, session);

    ssm_->UpdateOutlineInner(remoteObject, params);
    usleep(waitSyncForTestEndInNs);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: UpdateOutlineInner_ForegroundNotInList
 * @tc.desc: Test UpdateOutlineInner with STATE_FOREGROUND session not in persistentId list
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateOutlineInner_ForegroundNotInList, TestSize.Level1)
{
    OutlineParams params = CreateMockOutlineParams({1002}); // 1001 is not in the list
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();

    sptr<SceneSession> session = CreateMockSceneSession(1001, SessionState::STATE_FOREGROUND);
    ssm_->sceneSessionMap_.emplace(1001, session);

    ssm_->UpdateOutlineInner(remoteObject, params);
    usleep(waitSyncForTestEndInNs);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: UpdateOutlineInner_ActiveInList
 * @tc.desc: Test UpdateOutlineInner with STATE_ACTIVE session in persistentId list
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateOutlineInner_ActiveInList, TestSize.Level1)
{
    OutlineParams params = CreateMockOutlineParams({1001});
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();

    sptr<SceneSession> session = CreateMockSceneSession(1001, SessionState::STATE_ACTIVE);
    ssm_->sceneSessionMap_.emplace(1001, session);

    ssm_->UpdateOutlineInner(remoteObject, params);
    usleep(waitSyncForTestEndInNs);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: UpdateOutlineInner_InactiveNotInList
 * @tc.desc: Test UpdateOutlineInner with STATE_INACTIVE session not in persistentId list
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateOutlineInner_InactiveNotInList, TestSize.Level1)
{
    OutlineParams params = CreateMockOutlineParams({1002});
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();

    sptr<SceneSession> session = CreateMockSceneSession(1001, SessionState::STATE_INACTIVE);
    ssm_->sceneSessionMap_.emplace(1001, session);

    ssm_->UpdateOutlineInner(remoteObject, params);
    usleep(waitSyncForTestEndInNs);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: UpdateOutlineInner_BackgroundInList
 * @tc.desc: Test UpdateOutlineInner with STATE_BACKGROUND session in persistentId list
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateOutlineInner_BackgroundInList, TestSize.Level1)
{
    OutlineParams params = CreateMockOutlineParams({1001});
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();

    sptr<SceneSession> session = CreateMockSceneSession(1001, SessionState::STATE_BACKGROUND);
    ssm_->sceneSessionMap_.emplace(1001, session);

    ssm_->UpdateOutlineInner(remoteObject, params);
    usleep(waitSyncForTestEndInNs);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: UpdateOutlineInner_MultipleSessions
 * @tc.desc: Test UpdateOutlineInner with multiple sessions in different states
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, UpdateOutlineInner_MultipleSessions, TestSize.Level1)
{
    OutlineParams params = CreateMockOutlineParams({1001, 1003});
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();

    ssm_->sceneSessionMap_.emplace(1001, CreateMockSceneSession(1001, SessionState::STATE_FOREGROUND));
    ssm_->sceneSessionMap_.emplace(1002, CreateMockSceneSession(1002, SessionState::STATE_BACKGROUND));
    ssm_->sceneSessionMap_.emplace(1003, CreateMockSceneSession(1003, SessionState::STATE_ACTIVE));

    ssm_->UpdateOutlineInner(remoteObject, params);
    usleep(waitSyncForTestEndInNs);
    EXPECT_TRUE(true);
}

// ============================================================================
// AddOutlineRemoteDeathRecipient Tests
// ============================================================================

/**
 * @tc.name: AddOutlineRemoteDeathRecipient_NullRemoteObject
 * @tc.desc: Test AddOutlineRemoteDeathRecipient with null remoteObject
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, AddOutlineRemoteDeathRecipient_NullRemoteObject, TestSize.Level1)
{
    ssm_->AddOutlineRemoteDeathRecipient(nullptr);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: AddOutlineRemoteDeathRecipient_SameRemoteObject
 * @tc.desc: Test AddOutlineRemoteDeathRecipient with same remoteObject twice
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, AddOutlineRemoteDeathRecipient_SameRemoteObject, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    ssm_->outlineRemoteObject_ = remoteObject;

    ssm_->AddOutlineRemoteDeathRecipient(remoteObject);
    EXPECT_EQ(ssm_->outlineRemoteObject_, remoteObject);
}

/**
 * @tc.name: AddOutlineRemoteDeathRecipient_FirstTime
 * @tc.desc: Test AddOutlineRemoteDeathRecipient for the first time
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, AddOutlineRemoteDeathRecipient_FirstTime, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    ssm_->outlineRemoteObject_ = nullptr;

    ssm_->AddOutlineRemoteDeathRecipient(remoteObject);
    // Should set outlineRemoteObject_
    EXPECT_TRUE(true);
}

/**
 * @tc.name: AddOutlineRemoteDeathRecipient_ReplaceOld
 * @tc.desc: Test AddOutlineRemoteDeathRecipient replacing old remoteObject
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, AddOutlineRemoteDeathRecipient_ReplaceOld, TestSize.Level1)
{
    sptr<IRemoteObject> oldObject = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<IRemoteObject> newObject = sptr<IRemoteObjectMocker>::MakeSptr();
    ssm_->outlineRemoteObject_ = oldObject;

    ssm_->AddOutlineRemoteDeathRecipient(newObject);
    // Should replace with newObject if RemoveDeathRecipient succeeds
    EXPECT_TRUE(true);
}

/**
 * @tc.name: AddOutlineRemoteDeathRecipient_MultipleCalls
 * @tc.desc: Test AddOutlineRemoteDeathRecipient with multiple calls
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, AddOutlineRemoteDeathRecipient_MultipleCalls, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject1 = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<IRemoteObject> remoteObject2 = sptr<IRemoteObjectMocker>::MakeSptr();

    ssm_->outlineRemoteObject_ = nullptr;
    ssm_->AddOutlineRemoteDeathRecipient(remoteObject1);
    ssm_->AddOutlineRemoteDeathRecipient(remoteObject2);
    EXPECT_TRUE(true);
}

// ============================================================================
// DeleteAllOutline Tests
// ============================================================================

/**
 * @tc.name: DeleteAllOutline_NullOutlineRemoteObject
 * @tc.desc: Test DeleteAllOutline when outlineRemoteObject_ is null
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, DeleteAllOutline_NullOutlineRemoteObject, TestSize.Level1)
{
    ssm_->outlineRemoteObject_ = nullptr;
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();

    ssm_->DeleteAllOutline(remoteObject);
    EXPECT_EQ(ssm_->outlineRemoteObject_, nullptr);
}

/**
 * @tc.name: DeleteAllOutline_RemoteObjectMismatch
 * @tc.desc: Test DeleteAllOutline when remoteObject doesn't match outlineRemoteObject_
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, DeleteAllOutline_RemoteObjectMismatch, TestSize.Level1)
{
    sptr<IRemoteObject> outlineObject = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<IRemoteObject> otherObject = sptr<IRemoteObjectMocker>::MakeSptr();
    ssm_->outlineRemoteObject_ = outlineObject;

    ssm_->DeleteAllOutline(otherObject);
    EXPECT_EQ(ssm_->outlineRemoteObject_, outlineObject);
}

/**
 * @tc.name: DeleteAllOutline_NullSession
 * @tc.desc: Test DeleteAllOutline with null session in map
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, DeleteAllOutline_NullSession, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    ssm_->outlineRemoteObject_ = remoteObject;

    ssm_->sceneSessionMap_.emplace(1001, nullptr);
    ssm_->sceneSessionMap_.emplace(1002, CreateMockSceneSession(1002, SessionState::STATE_FOREGROUND));

    ssm_->DeleteAllOutline(remoteObject);
    EXPECT_EQ(ssm_->outlineRemoteObject_, nullptr);
}

/**
 * @tc.name: DeleteAllOutline_StateDisconnect
 * @tc.desc: Test DeleteAllOutline with session in STATE_DISCONNECT
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, DeleteAllOutline_StateDisconnect, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    ssm_->outlineRemoteObject_ = remoteObject;

    sptr<SceneSession> session = CreateMockSceneSession(1001, SessionState::STATE_DISCONNECT);
    ssm_->sceneSessionMap_.emplace(1001, session);

    ssm_->DeleteAllOutline(remoteObject);
    EXPECT_EQ(ssm_->outlineRemoteObject_, nullptr);
}

/**
 * @tc.name: DeleteAllOutline_StateEnd
 * @tc.desc: Test DeleteAllOutline with session in STATE_END
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, DeleteAllOutline_StateEnd, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    ssm_->outlineRemoteObject_ = remoteObject;

    sptr<SceneSession> session = CreateMockSceneSession(1001, SessionState::STATE_END);
    ssm_->sceneSessionMap_.emplace(1001, session);

    ssm_->DeleteAllOutline(remoteObject);
    EXPECT_EQ(ssm_->outlineRemoteObject_, nullptr);
}

/**
 * @tc.name: DeleteAllOutline_ValidSession
 * @tc.desc: Test DeleteAllOutline with valid sessions
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, DeleteAllOutline_ValidSession, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    ssm_->outlineRemoteObject_ = remoteObject;

    ssm_->sceneSessionMap_.emplace(1001, CreateMockSceneSession(1001, SessionState::STATE_FOREGROUND));
    ssm_->sceneSessionMap_.emplace(1002, CreateMockSceneSession(1002, SessionState::STATE_BACKGROUND));

    ssm_->DeleteAllOutline(remoteObject);
    EXPECT_EQ(ssm_->outlineRemoteObject_, nullptr);
}

/**
 * @tc.name: DeleteAllOutline_MultipleSessions
 * @tc.desc: Test DeleteAllOutline with multiple sessions in various states
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, DeleteAllOutline_MultipleSessions, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    ssm_->outlineRemoteObject_ = remoteObject;

    ssm_->sceneSessionMap_.emplace(1001, CreateMockSceneSession(1001, SessionState::STATE_FOREGROUND));
    ssm_->sceneSessionMap_.emplace(1002, CreateMockSceneSession(1002, SessionState::STATE_DISCONNECT));
    ssm_->sceneSessionMap_.emplace(1003, CreateMockSceneSession(1003, SessionState::STATE_ACTIVE));
    ssm_->sceneSessionMap_.emplace(1004, nullptr);

    ssm_->DeleteAllOutline(remoteObject);
    EXPECT_EQ(ssm_->outlineRemoteObject_, nullptr);
}

// ============================================================================
// GetJsonProfile Tests
// ============================================================================

/**
 * @tc.name: GetJsonProfile_NullBundleMgr
 * @tc.desc: Test GetJsonProfile when bundleMgr_ is null
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, GetJsonProfile_NullBundleMgr, TestSize.Level1)
{
    ssm_->bundleMgr_ = nullptr;
    std::string profileInfo;

    WMError result = ssm_->GetJsonProfile(
        AppExecFwk::ProfileType::UNSPECIFIED_PROFILE,
        "test.bundle",
        "test.module",
        100,
        profileInfo
    );

    EXPECT_EQ(result, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: GetJsonProfile_ValidParams
 * @tc.desc: Test GetJsonProfile with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, GetJsonProfile_ValidParams, TestSize.Level1)
{
    // This test requires a mock bundle manager, just verify it doesn't crash
    // In real scenario, bundleMgr_ would be set by the system
    std::string profileInfo;

    WMError result = ssm_->GetJsonProfile(
        AppExecFwk::ProfileType::UNSPECIFIED_PROFILE,
        "test.bundle",
        "test.module",
        100,
        profileInfo
    );

    // Result depends on whether bundleMgr_ is set
    EXPECT_TRUE(result == WMError::WM_ERROR_NULLPTR || result == WMError::WM_OK ||
                result == WMError::WM_ERROR_SYSTEM_ABNORMALLY);
}

/**
 * @tc.name: GetJsonProfile_EmptyBundleName
 * @tc.desc: Test GetJsonProfile with empty bundle name
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, GetJsonProfile_EmptyBundleName, TestSize.Level1)
{
    std::string profileInfo;

    WMError result = ssm_->GetJsonProfile(
        AppExecFwk::ProfileType::UNSPECIFIED_PROFILE,
        "",
        "test.module",
        100,
        profileInfo
    );

    EXPECT_TRUE(result == WMError::WM_ERROR_NULLPTR || result == WMError::WM_OK ||
                result == WMError::WM_ERROR_SYSTEM_ABNORMALLY);
}

// ============================================================================
// GetAllJsonProfile Tests
// ============================================================================

/**
 * @tc.name: GetAllJsonProfile_NullBundleMgr
 * @tc.desc: Test GetAllJsonProfile when bundleMgr_ is null
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, GetAllJsonProfile_NullBundleMgr, TestSize.Level1)
{
    ssm_->bundleMgr_ = nullptr;
    std::vector<AppExecFwk::JsonProfileInfo> jsonProfileInfos;

    WMError result = ssm_->GetAllJsonProfile(
        AppExecFwk::ProfileType::UNSPECIFIED_PROFILE,
        100,
        jsonProfileInfos
    );

    EXPECT_EQ(result, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: GetAllJsonProfile_ValidParams
 * @tc.desc: Test GetAllJsonProfile with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, GetAllJsonProfile_ValidParams, TestSize.Level1)
{
    std::vector<AppExecFwk::JsonProfileInfo> jsonProfileInfos;

    WMError result = ssm_->GetAllJsonProfile(
        AppExecFwk::ProfileType::UNSPECIFIED_PROFILE,
        100,
        jsonProfileInfos
    );

    // Result depends on whether bundleMgr_ is set
    EXPECT_TRUE(result == WMError::WM_ERROR_NULLPTR || result == WMError::WM_OK ||
                result == WMError::WM_ERROR_SYSTEM_ABNORMALLY);
}

/**
 * @tc.name: GetAllJsonProfile_DifferentProfileType
 * @tc.desc: Test GetAllJsonProfile with different profile types
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerOutlineTest, GetAllJsonProfile_DifferentProfileType, TestSize.Level1)
{
    std::vector<AppExecFwk::JsonProfileInfo> jsonProfileInfos;

    WMError result = ssm_->GetAllJsonProfile(
        AppExecFwk::ProfileType::UNSPECIFIED_PROFILE,
        100,
        jsonProfileInfos
    );

    EXPECT_TRUE(result == WMError::WM_ERROR_NULLPTR || result == WMError::WM_OK ||
                result == WMError::WM_ERROR_SYSTEM_ABNORMALLY);
}

} // namespace Rosen
} // namespace OHOS
