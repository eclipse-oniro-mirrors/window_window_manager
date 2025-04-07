/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "interfaces/include/ws_common.h"
#include "mock/mock_session_stage.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "session_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class SceneSessionManagerTest8 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
private:
    sptr<SceneSessionManager> ssm_;
    static constexpr uint32_t WAIT_SYNC_IN_NS = 500000;
};

void SceneSessionManagerTest8::SetUpTestCase()
{
}

void SceneSessionManagerTest8::TearDownTestCase()
{
}

void SceneSessionManagerTest8::SetUp()
{
    ssm_ = &SceneSessionManager::GetInstance();
    EXPECT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerTest8::TearDown()
{
    ssm_->sceneSessionMap_.clear();
    usleep(WAIT_SYNC_IN_NS);
    ssm_ = nullptr;
}

namespace {
/**
 * @tc.name: GetTotalUITreeInfo
 * @tc.desc: GetTotalUITreeInfo set gesture navigation enabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, GetTotalUITreeInfo, Function | SmallTest | Level3)
{
    std::string dumpInfo = "dumpInfo";
    ssm_->SetDumpUITreeFunc(nullptr);
    EXPECT_EQ(WSError::WS_OK, ssm_->GetTotalUITreeInfo(dumpInfo));
    DumpUITreeFunc func = [](std::string& dumpInfo) {
        return;
    };
    ssm_->SetDumpUITreeFunc(func);
    EXPECT_EQ(WSError::WS_OK, ssm_->GetTotalUITreeInfo(dumpInfo));
}

/**
 * @tc.name: GetRemoteSessionSnapshotInfo
 * @tc.desc: GetRemoteSessionSnapshotInfo set gesture navigation enabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, GetRemoteSessionSnapshotInfo, Function | SmallTest | Level3)
{
    AAFwk::MissionSnapshot sessionSnapshot;
    std::string deviceId = "";
    int res = ssm_->GetRemoteSessionSnapshotInfo(deviceId, 8, sessionSnapshot);
    EXPECT_EQ(ERR_NULL_OBJECT, res);
}

/**
 * @tc.name: WindowLayerInfoChangeCallback
 * @tc.desc: test function : WindowLayerInfoChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, WindowLayerInfoChangeCallback, Function | SmallTest | Level3)
{
    std::shared_ptr<RSOcclusionData> rsData = nullptr;
    ssm_->WindowLayerInfoChangeCallback(rsData);

    rsData = std::make_shared<RSOcclusionData>();
    ASSERT_NE(nullptr, rsData);
    ssm_->WindowLayerInfoChangeCallback(rsData);

    VisibleData visibleData;
    visibleData.push_back(std::make_pair(0, WINDOW_LAYER_INFO_TYPE::ALL_VISIBLE));
    visibleData.push_back(std::make_pair(1, WINDOW_LAYER_INFO_TYPE::SEMI_VISIBLE));
    visibleData.push_back(std::make_pair(2, WINDOW_LAYER_INFO_TYPE::INVISIBLE));
    visibleData.push_back(std::make_pair(3, WINDOW_LAYER_INFO_TYPE::WINDOW_LAYER_DYNAMIC_STATUS));
    visibleData.push_back(std::make_pair(4, WINDOW_LAYER_INFO_TYPE::WINDOW_LAYER_STATIC_STATUS));
    visibleData.push_back(std::make_pair(5, WINDOW_LAYER_INFO_TYPE::WINDOW_LAYER_UNKNOWN_TYPE));
    rsData = std::make_shared<RSOcclusionData>(visibleData);
    ASSERT_NE(nullptr, rsData);
    ssm_->WindowLayerInfoChangeCallback(rsData);
}

/**
 * @tc.name: PostProcessFocus
 * @tc.desc: test function : PostProcessFocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, PostProcessFocus, Function | SmallTest | Level3)
{
    ssm_->sceneSessionMap_.emplace(0, nullptr);
    ssm_->PostProcessFocus();
    ssm_->sceneSessionMap_.clear();

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessFocus";
    sessionInfo.abilityName_ = "PostProcessFocus";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    PostProcessFocusState state;
    EXPECT_EQ(false, state.enabled_);
    sceneSession->SetPostProcessFocusState(state);
    ssm_->sceneSessionMap_.emplace(0, sceneSession);
    ssm_->PostProcessFocus();

    state.enabled_ = true;
    state.isFocused_ = false;
    sceneSession->SetPostProcessFocusState(state);
    ssm_->PostProcessFocus();

    state.isFocused_ = true;
    state.reason_ = FocusChangeReason::SCB_START_APP;
    sceneSession->SetPostProcessFocusState(state);
    ssm_->PostProcessFocus();

    sceneSession->SetPostProcessFocusState(state);
    state.reason_ = FocusChangeReason::DEFAULT;
    ssm_->PostProcessFocus();
}

/**
 * @tc.name: PostProcessFocus01
 * @tc.desc: test function : PostProcessFocus with focusableOnShow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, PostProcessFocus01, Function | SmallTest | Level3)
{
    ssm_->sceneSessionMap_.clear();
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(0);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessFocus01";
    sessionInfo.abilityName_ = "PostProcessFocus01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->isVisible_ = true;

    PostProcessFocusState state = {true, true, true, FocusChangeReason::FOREGROUND};
    sceneSession->SetPostProcessFocusState(state);
    sceneSession->SetFocusableOnShow(false);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    ssm_->PostProcessFocus();

    EXPECT_NE(1, focusGroup->GetFocusedSessionId());
}

/**
 * @tc.name: PostProcessFocus03
 * @tc.desc: test function : PostProcessFocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, PostProcessFocus03, Function | SmallTest | Level3)
{
    ssm_->sceneSessionMap_.clear();

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessFocus03";
    sessionInfo.abilityName_ = "PostProcessFocus03";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->persistentId_ = 1;

    sceneSession->SetFocusedOnShow(false);
    PostProcessFocusState state = {true, true, true, FocusChangeReason::FOREGROUND};
    sceneSession->SetPostProcessFocusState(state);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    ssm_->PostProcessFocus();
    EXPECT_EQ(sceneSession->IsFocusedOnShow(), false);

    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->isVisible_ = true;
    state = {true, true, true, FocusChangeReason::FOREGROUND};
    sceneSession->SetPostProcessFocusState(state);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    ssm_->PostProcessFocus();
    EXPECT_EQ(sceneSession->IsFocusedOnShow(), true);
}

/**
 * @tc.name: PostProcessProperty
 * @tc.desc: test function : PostProcessProperty
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, PostProcessProperty, Function | SmallTest | Level3)
{
    ssm_->sceneSessionMap_.emplace(0, nullptr);
    ssm_->PostProcessProperty(static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA));
    ssm_->PostProcessProperty(~static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA));
    ssm_->sceneSessionMap_.clear();

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessProperty";
    sessionInfo.abilityName_ = "PostProcessProperty";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_DIALOG);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    PostProcessFocusState state;
    EXPECT_EQ(false, state.enabled_);
    sceneSession->SetPostProcessFocusState(state);
    ssm_->sceneSessionMap_.emplace(0, sceneSession);
    ssm_->PostProcessFocus();

    state.enabled_ = true;
    sceneSession->SetPostProcessFocusState(state);
    ssm_->PostProcessFocus();

    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->PostProcessFocus();
}

/**
 * @tc.name: NotifyUpdateRectAfterLayout
 * @tc.desc: test function : NotifyUpdateRectAfterLayout
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, NotifyUpdateRectAfterLayout, Function | SmallTest | Level3)
{
    ssm_->sceneSessionMap_.emplace(0, nullptr);
    ssm_->NotifyUpdateRectAfterLayout();
    ssm_->sceneSessionMap_.clear();

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "NotifyUpdateRectAfterLayout";
    sessionInfo.abilityName_ = "NotifyUpdateRectAfterLayout";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.emplace(0, sceneSession);
    ssm_->NotifyUpdateRectAfterLayout();
    constexpr uint32_t NOT_WAIT_SYNC_IN_NS = 500000;
    usleep(NOT_WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: DestroyExtensionSession
 * @tc.desc: test function : DestroyExtensionSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, DestroyExtensionSession, Function | SmallTest | Level3)
{
    ssm_->remoteExtSessionMap_.clear();
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<IRemoteObject> token = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_NE(nullptr, iRemoteObjectMocker);
    ssm_->DestroyExtensionSession(iRemoteObjectMocker);
    ssm_->remoteExtSessionMap_.emplace(iRemoteObjectMocker, token);

    ssm_->extSessionInfoMap_.clear();
    ssm_->DestroyExtensionSession(iRemoteObjectMocker);

    ExtensionWindowAbilityInfo extensionWindowAbilituInfo;
    ssm_->extSessionInfoMap_.emplace(token, extensionWindowAbilituInfo);

    ssm_->sceneSessionMap_.emplace(0, nullptr);
    ssm_->DestroyExtensionSession(iRemoteObjectMocker);
    ssm_->sceneSessionMap_.clear();

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "DestroyExtensionSession";
    sessionInfo.abilityName_ = "DestroyExtensionSession";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.emplace(0, sceneSession);

    ExtensionWindowFlags extensionWindowFlags;
    sceneSession->combinedExtWindowFlags_ = extensionWindowFlags;
    ssm_->DestroyExtensionSession(iRemoteObjectMocker);

    extensionWindowFlags.waterMarkFlag = false;
    extensionWindowFlags.privacyModeFlag = false;
    sceneSession->combinedExtWindowFlags_ = extensionWindowFlags;
    EXPECT_EQ(false, sceneSession->combinedExtWindowFlags_.privacyModeFlag);
    int len = sceneSession->modalUIExtensionInfoList_.size();
    ssm_->DestroyExtensionSession(iRemoteObjectMocker, true);
    constexpr uint32_t DES_WAIT_SYNC_IN_NS = 500000;
    usleep(DES_WAIT_SYNC_IN_NS);
    EXPECT_EQ(len, sceneSession->modalUIExtensionInfoList_.size());
    ssm_->DestroyExtensionSession(iRemoteObjectMocker, false);
    usleep(DES_WAIT_SYNC_IN_NS);
    EXPECT_EQ(len, sceneSession->modalUIExtensionInfoList_.size());
}

/**
 * @tc.name: FilterSceneSessionCovered
 * @tc.desc: test function : FilterSceneSessionCovered
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, FilterSceneSessionCovered, Function | SmallTest | Level3)
{
    std::vector<sptr<SceneSession>> sceneSessionList;
    sptr<SceneSession> sceneSession = nullptr;
    sceneSessionList.emplace_back(sceneSession);
    EXPECT_EQ(1, sceneSessionList.size());
    ssm_->FilterSceneSessionCovered(sceneSessionList);

    SessionInfo sessionInfo;
    sceneSessionList.clear();
    sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    sceneSessionList.emplace_back(sceneSession);
    ssm_->FilterSceneSessionCovered(sceneSessionList);
}

/**
 * @tc.name: UpdateSubWindowVisibility
 * @tc.desc: test function : UpdateSubWindowVisibility
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, UpdateSubWindowVisibility, Function | SmallTest | Level3)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "UpdateSubWindowVisibility";
    sessionInfo.abilityName_ = "UpdateSubWindowVisibility";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::APP_SUB_WINDOW_BASE);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    WindowVisibilityState visibleState = WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION;
    std::vector<std::pair<uint64_t, WindowVisibilityState>> visibilityChangeInfo;
    std::vector<sptr<WindowVisibilityInfo>> windowVisibilityInfos;
    std::string visibilityInfo = "";
    std::vector<std::pair<uint64_t, WindowVisibilityState>> currVisibleData;
    sceneSession->persistentId_ = 1998;
    sceneSession->SetCallingUid(1998);
    SessionState state = SessionState::STATE_CONNECT;
    sceneSession->SetSessionState(state);
    sceneSession->SetParentSession(sceneSession);
    EXPECT_EQ(1998, sceneSession->GetParentSession()->GetWindowId());
    ssm_->sceneSessionMap_.emplace(0, sceneSession);

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    EXPECT_NE(nullptr, sceneSession1);
    sceneSession1->persistentId_ = 1998;
    sceneSession1->SetCallingUid(1024);
    SessionState state1 = SessionState::STATE_CONNECT;
    sceneSession1->SetSessionState(state1);
    sceneSession1->SetParentSession(sceneSession1);
    EXPECT_EQ(1998, sceneSession1->GetParentSession()->GetWindowId());
    ssm_->sceneSessionMap_.emplace(0, sceneSession1);

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    EXPECT_NE(nullptr, sceneSession2);
    sceneSession2->persistentId_ = 1998;
    sceneSession2->SetCallingUid(1998);
    SessionState state2 = SessionState::STATE_FOREGROUND;
    sceneSession2->SetSessionState(state2);
    sceneSession2->SetParentSession(sceneSession2);
    EXPECT_EQ(1998, sceneSession2->GetParentSession()->GetWindowId());
    ssm_->sceneSessionMap_.emplace(0, sceneSession2);
    ssm_->UpdateSubWindowVisibility(sceneSession,
        visibleState, visibilityChangeInfo, windowVisibilityInfos, visibilityInfo, currVisibleData);
}

/**
 * @tc.name: RegisterSessionChangeByActionNotifyManagerFunc
 * @tc.desc: test function : RegisterSessionChangeByActionNotifyManagerFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, RegisterSessionChangeByActionNotifyManagerFunc, Function | SmallTest | Level3)
{
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->RegisterSessionChangeByActionNotifyManagerFunc(sceneSession);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "RegisterSessionChangeByActionNotifyManagerFunc";
    sessionInfo.abilityName_ = "RegisterSessionChangeByActionNotifyManagerFunc";
    sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    ssm_->RegisterSessionChangeByActionNotifyManagerFunc(sceneSession);
    EXPECT_NE(nullptr, sceneSession->sessionChangeByActionNotifyManagerFunc_);

    sptr<WindowSessionProperty> property = nullptr;
    sceneSession->NotifySessionChangeByActionNotifyManager(property,
        WSPropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON);

    property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(nullptr, property);

    sceneSession->NotifySessionChangeByActionNotifyManager(property,
        WSPropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON);
}

/**
 * @tc.name: RegisterSessionChangeByActionNotifyManagerFunc1
 * @tc.desc: test function : RegisterSessionChangeByActionNotifyManagerFunc1
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, RegisterSessionChangeByActionNotifyManagerFunc1, Function | SmallTest | Level3)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "RegisterSessionChangeByActionNotifyManagerFunc1";
    sessionInfo.abilityName_ = "RegisterSessionChangeByActionNotifyManagerFunc1";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    EXPECT_NE(nullptr, sceneSession);

    ssm_->RegisterSessionChangeByActionNotifyManagerFunc(sceneSession);
    EXPECT_NE(nullptr, sceneSession->sessionChangeByActionNotifyManagerFunc_);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(nullptr, property);

    sceneSession->NotifySessionChangeByActionNotifyManager(property,
        WSPropertyChangeAction::ACTION_UPDATE_KEEP_SCREEN_ON);

    sceneSession->NotifySessionChangeByActionNotifyManager(property,
        WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_INDICATOR_PROPS);

    sceneSession->NotifySessionChangeByActionNotifyManager(property,
        WSPropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS);

    sceneSession->NotifySessionChangeByActionNotifyManager(property,
        WSPropertyChangeAction::ACTION_UPDATE_SYSTEM_PRIVACY_MODE);

    sceneSession->NotifySessionChangeByActionNotifyManager(property,
        WSPropertyChangeAction::ACTION_UPDATE_FLAGS);

    sceneSession->NotifySessionChangeByActionNotifyManager(property,
        WSPropertyChangeAction::ACTION_UPDATE_MODE);

    sceneSession->NotifySessionChangeByActionNotifyManager(property,
        WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS);

    sceneSession->NotifySessionChangeByActionNotifyManager(property,
        WSPropertyChangeAction::ACTION_UPDATE_WINDOW_MASK);

    sceneSession->NotifySessionChangeByActionNotifyManager(property,
        WSPropertyChangeAction::ACTION_UPDATE_TOPMOST);
}

/**
 * @tc.name: RegisterRequestFocusStatusNotifyManagerFunc
 * @tc.desc: test function : RegisterRequestFocusStatusNotifyManagerFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, RegisterRequestFocusStatusNotifyManagerFunc, Function | SmallTest | Level3)
{
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->RegisterRequestFocusStatusNotifyManagerFunc(sceneSession);
    EXPECT_EQ(nullptr, sceneSession);
}

/**
 * @tc.name: CheckRequestFocusImmdediately
 * @tc.desc: test function : CheckRequestFocusImmdediately
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, CheckRequestFocusImmdediately, Function | SmallTest | Level3)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CheckRequestFocusImmdediately";
    sessionInfo.abilityName_ = "CheckRequestFocusImmdediately";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::APP_SUB_WINDOW_BASE);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    EXPECT_EQ(WindowType::APP_SUB_WINDOW_BASE, sceneSession->GetWindowType());
    bool ret = ssm_->CheckRequestFocusImmdediately(sceneSession);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: HandleTurnScreenOn
 * @tc.desc: test function : HandleTurnScreenOn
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, HandleTurnScreenOn, Function | SmallTest | Level3)
{
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->HandleTurnScreenOn(sceneSession);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "HandleTurnScreenOn";
    sessionInfo.abilityName_ = "HandleTurnScreenOn";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::APP_SUB_WINDOW_BASE);
    sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    sceneSession->GetSessionProperty()->SetTurnScreenOn(false);
    ssm_->HandleTurnScreenOn(sceneSession);
    EXPECT_EQ(false, sceneSession->GetSessionProperty()->IsTurnScreenOn());
    sceneSession->GetSessionProperty()->SetTurnScreenOn(true);
    ssm_->HandleTurnScreenOn(sceneSession);
    constexpr uint32_t NOT_WAIT_SYNC_IN_NS = 500000;
    usleep(NOT_WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: HandleKeepScreenOn
 * @tc.desc: test function : HandleKeepScreenOn
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, HandleKeepScreenOn, Function | SmallTest | Level3)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "HandleTurnScreenOn";
    sessionInfo.abilityName_ = "HandleTurnScreenOn";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::APP_SUB_WINDOW_BASE);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    EXPECT_NE(nullptr, sceneSession);

    std::string lockName = "windowLock";
    ssm_->HandleKeepScreenOn(sceneSession, false, lockName, sceneSession->keepScreenLock_);
    sceneSession->keepScreenLock_ = nullptr;
    ssm_->HandleKeepScreenOn(sceneSession, true, lockName, sceneSession->keepScreenLock_);
    bool enable = true;
    EXPECT_EQ(WSError::WS_OK, ssm_->GetFreeMultiWindowEnableState(enable));
}

/**
 * @tc.name: SetBrightness
 * @tc.desc: test function : SetBrightness
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, SetBrightness, Function | SmallTest | Level3)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SetBrightness";
    sessionInfo.abilityName_ = "SetBrightness";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    sceneSession->persistentId_ = 2024;

    ssm_->SetDisplayBrightness(3.14f);
    std::shared_ptr<AppExecFwk::EventHandler> pipeEventHandler = nullptr;
    ssm_->eventHandler_ = pipeEventHandler;
    ASSERT_EQ(nullptr, ssm_->eventHandler_);
    auto ret = ssm_->SetBrightness(sceneSession, 3.15f);
    EXPECT_EQ(WSError::WS_OK, ret);

    ssm_->Init();
    ASSERT_NE(nullptr, ssm_->eventHandler_);

    ssm_->SetFocusedSessionId(2024, DEFAULT_DISPLAY_ID);
    EXPECT_EQ(2024, ssm_->GetFocusedSessionId());

    ret = ssm_->SetBrightness(sceneSession, 3.15f);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_EQ(3.15f, ssm_->GetDisplayBrightness());

    ret = ssm_->SetBrightness(sceneSession, UNDEFINED_BRIGHTNESS);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_EQ(UNDEFINED_BRIGHTNESS, ssm_->GetDisplayBrightness());
}

/**
 * @tc.name: TerminateSessionNew
 * @tc.desc: test function : TerminateSessionNew
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, TerminateSessionNew, Function | SmallTest | Level3)
{
    sptr<AAFwk::SessionInfo> sessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    EXPECT_NE(nullptr, sessionInfo);
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_NE(nullptr, iRemoteObjectMocker);
    sessionInfo->sessionToken = iRemoteObjectMocker;

    SessionInfo info;
    info.bundleName_ = "TerminateSessionNew";
    info.abilityName_ = "TerminateSessionNew";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    sceneSession->SetAbilityToken(iRemoteObjectMocker);
    ssm_->sceneSessionMap_.emplace(0, sceneSession);
    ssm_->TerminateSessionNew(sessionInfo, true, true);
}

/**
 * @tc.name: IsLastFrameLayoutFinished
 * @tc.desc: test function : IsLastFrameLayoutFinished
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, IsLastFrameLayoutFinished, Function | SmallTest | Level3)
{
    ssm_->closeTargetFloatWindowFunc_ = nullptr;
    std::string bundleName = "SetCloseTargetFloatWindowFunc";
    ProcessCloseTargetFloatWindowFunc func = [](const std::string& bundleName1) {
        return ;
    };
    ssm_->SetCloseTargetFloatWindowFunc(func);

    IsRootSceneLastFrameLayoutFinishedFunc func1 = []() {
        return true;
    };
    ssm_->isRootSceneLastFrameLayoutFinishedFunc_ = func1;
    ASSERT_NE(ssm_->isRootSceneLastFrameLayoutFinishedFunc_, nullptr);
    bool isLayoutFinished = false;
    auto ret = ssm_->IsLastFrameLayoutFinished(isLayoutFinished);
    EXPECT_EQ(true, isLayoutFinished);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: ReportScreenFoldStatus
 * @tc.desc: test function : ReportScreenFoldStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, ReportScreenFoldStatus, Function | SmallTest | Level3)
{
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->sceneSessionMap_.insert(std::make_pair(0, sceneSession));
    SessionInfo info;
    info.bundleName_ = "ReportScreenFoldStatus";
    info.abilityName_ = "ReportScreenFoldStatus";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    sceneSession1->SetSessionState(SessionState::STATE_FOREGROUND);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession1));
    SessionInfo info1;
    info1.bundleName_ = "ReportScreenFoldStatus1";
    info1.abilityName_ = "ReportScreenFoldStatus1";
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    sceneSession2->SetSessionState(SessionState::STATE_ACTIVE);
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession2));
    SessionInfo info2;
    info2.bundleName_ = "ReportScreenFoldStatus2";
    info2.abilityName_ = "ReportScreenFoldStatus2";
    sptr<SceneSession> sceneSession3 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    ASSERT_NE(sceneSession3, nullptr);
    sceneSession3->SetSessionState(SessionState::STATE_BACKGROUND);
    ssm_->sceneSessionMap_.insert(std::make_pair(3, sceneSession3));
    ssm_->OnScreenshot(1);
    constexpr uint32_t NOT_WAIT_SYNC_IN_NS = 500000;
    usleep(NOT_WAIT_SYNC_IN_NS);

    ScreenFoldData data;
    data.currentScreenFoldStatus_ = ScreenFoldData::INVALID_VALUE;
    auto ret = ssm_->ReportScreenFoldStatus(data);
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);
}

/**
 * @tc.name: GetWindowModeType
 * @tc.desc: test function : GetWindowModeType
 * @tc.type: FUNC
 */

HWTEST_F(SceneSessionManagerTest8, GetWindowModeType, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.bundleName_ = "GetWindowModeType";
    info.abilityName_ = "GetWindowModeType";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->NotifySessionBackground(sceneSession, 1, true, true);
    WindowModeType windowModeType = WindowModeType::WINDOW_MODE_SPLIT_FLOATING;
    auto ret = ssm_->GetWindowModeType(windowModeType);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);
}

/**
 * @tc.name: GetHostWindowRect
 * @tc.desc: test function : GetHostWindowRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, GetHostWindowRect, Function | SmallTest | Level3)
{
    sptr<IDisplayChangeListener> listener = sptr<DisplayChangeListener>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    DisplayId displayId = 1;
    listener->OnScreenshot(displayId);
    constexpr uint32_t NOT_WAIT_SYNC_IN_NS = 500000;
    usleep(NOT_WAIT_SYNC_IN_NS);

    int32_t hostWindowId = 0;
    Rect rect = { 0, 0, 0, 0 };
    SessionInfo info;
    info.bundleName_ = "GetHostWindowRect";
    info.abilityName_ = "GetHostWindowRect";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->sessionInfo_.screenId_ = 0;
    EXPECT_EQ(sceneSession->GetScreenId(), 0);
    ssm_->sceneSessionMap_.insert(std::make_pair(hostWindowId, sceneSession));
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(0, SuperFoldStatus::EXPANDED,
        { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    auto ret = ssm_->GetHostWindowRect(hostWindowId, rect);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_EQ(rect.posY_, 0);
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(0, SuperFoldStatus::KEYBOARD,
        { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    sceneSession->winRect_ = {0, 100, 0, 0};
    ret = ssm_->GetHostWindowRect(hostWindowId, rect);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_EQ(rect.posY_, 100);

    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(0, SuperFoldStatus::HALF_FOLDED,
        { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1649, 2472, 40 });
    sceneSession->winRect_ = {0, 1000, 100, 100};
    ret = ssm_->GetHostWindowRect(hostWindowId, rect);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_EQ(rect.posY_, 1000);
    sceneSession->winRect_ = {0, 2000, 100, 100};
    ret = ssm_->GetHostWindowRect(hostWindowId, rect);
    WSRect hostRect = {0, 2000, 100, 100};
    sceneSession->TransformGlobalRectToRelativeRect(hostRect);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_EQ(rect.posY_, hostRect.posY_);

    sceneSession->GetSessionProperty()->SetIsSystemKeyboard(false);
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(0, SuperFoldStatus::UNKNOWN,
        { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    sceneSession->winRect_ = {0, 0, 0, 0};
    ret = ssm_->GetHostWindowRect(hostWindowId, rect);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_EQ(rect.posY_, 0);
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(0, SuperFoldStatus::FOLDED,
        { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    sceneSession->winRect_ = {0, 100, 0, 0};
    ret = ssm_->GetHostWindowRect(hostWindowId, rect);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_EQ(rect.posY_, 100);

    sceneSession->GetSessionProperty()->SetIsSystemKeyboard(true);
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(0, SuperFoldStatus::HALF_FOLDED,
        { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1649, 2472, 40 });
    sceneSession->winRect_ = {0, 1000, 100, 100};
    ret = ssm_->GetHostWindowRect(hostWindowId, rect);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_EQ(rect.posY_, 1000);
}

/**
 * @tc.name: NotifyStackEmpty
 * @tc.desc: test function : NotifyStackEmpty
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, NotifyStackEmpty, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.bundleName_ = "NotifyStackEmpty";
    info.abilityName_ = "NotifyStackEmpty";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    auto ret = ssm_->NotifyStackEmpty(0);
    EXPECT_EQ(ret, WSError::WS_OK);
    constexpr uint32_t NOT_WAIT_SYNC_IN_NS = 500000;
    usleep(NOT_WAIT_SYNC_IN_NS);
    ret = ssm_->NotifyStackEmpty(1);
    EXPECT_EQ(WSError::WS_OK, ret);
    usleep(NOT_WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: GetAppMainSceneSession
 * @tc.desc: test function : GetAppMainSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, GetAppMainSceneSession, Function | SmallTest | Level3)
{
    ssm_->isUserBackground_ = true;
    ssm_->FlushWindowInfoToMMI(true);

    SessionInfo info;
    info.bundleName_ = "GetAppMainSceneSession";
    info.abilityName_ = "GetAppMainSceneSession";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    property->SetParentPersistentId(2);
    sceneSession->property_ = property;
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    auto ret = ssm_->GetAppMainSceneSession(1, sceneSession);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_SESSION, ret);
}

/**
 * @tc.name: PostProcessProperty01
 * @tc.desc: test function : PostProcessProperty
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, PostProcessProperty01, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.bundleName_ = "PostProcessProperty";
    info.abilityName_ = "PostProcessProperty";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->property_ = property;
    sceneSession->postProcessProperty_ = true;
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    uint32_t dirty = static_cast<uint32_t>(SessionUIDirtyFlag::AVOID_AREA);
    ssm_->PostProcessProperty(dirty);

    dirty = static_cast<uint32_t>(SessionUIDirtyFlag::VISIBLE);
    ssm_->PostProcessProperty(dirty);

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ssm_->PostProcessProperty(dirty);
    EXPECT_EQ(false, sceneSession->postProcessProperty_);
}

/**
 * @tc.name: SetVmaCacheStatus
 * @tc.desc: test function : SetVmaCacheStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, SetVmaCacheStatus, Function | SmallTest | Level3)
{
    AppExecFwk::AbilityInfo abilityInfo;
    ssm_->ProcessPreload(abilityInfo);

    auto ret = ssm_->SetVmaCacheStatus(true);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: IsInDefaultScreen
 * @tc.desc: test function : IsInDefaultScreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, IsInDefaultScreen, Function | SmallTest | Level3)
{
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->ProcessFocusWhenForegroundScbCore(sceneSession);

    SessionInfo info;
    info.bundleName_ = "IsInDefaultScreen";
    info.abilityName_ = "IsInDefaultScreen";
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    auto ret = ssm_->IsInDefaultScreen(sceneSession);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: OnSessionStateChange
 * @tc.desc: test function : OnSessionStateChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, OnSessionStateChange, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.bundleName_ = "OnSessionStateChange";
    info.abilityName_ = "OnSessionStateChange";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->property_ = property;
    SessionState state = SessionState::STATE_DISCONNECT;
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert(std::make_pair(100, sceneSession));
    ssm_->OnSessionStateChange(100, state);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ssm_->OnSessionStateChange(100, state);

    ssm_->isRootSceneLastFrameLayoutFinishedFunc_ = nullptr;
    bool isLayoutFinished = false;
    auto ret = ssm_->IsLastFrameLayoutFinished(isLayoutFinished);
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: OnSessionStateChange01
 * @tc.desc: test function : OnSessionStateChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, OnSessionStateChange01, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.bundleName_ = "OnSessionStateChange01";
    info.abilityName_ = "OnSessionStateChange01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->property_ = property;
    sceneSession->isScbCoreEnabled_ = true;
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    SessionState state = SessionState::STATE_FOREGROUND;
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert(std::make_pair(100, sceneSession));
    ssm_->OnSessionStateChange(100, state);

    property->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ssm_->OnSessionStateChange(100, state);
    auto ret = ssm_->UpdateMaximizeMode(1, true);
    EXPECT_EQ(WSError::WS_OK, ret);
    constexpr uint32_t NOT_WAIT_SYNC_IN_NS = 500000;
    usleep(NOT_WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: IsWindowSupportCacheForRecovering
 * @tc.desc: test function : IsWindowSupportCacheForRecovering
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, IsWindowSupportCacheForRecovering, Function | SmallTest | Level3)
{
    std::vector<int32_t> recoveredPersistentIds = {1};
    ssm_->alivePersistentIds_.clear();
    ssm_->alivePersistentIds_.push_back(1);
    ssm_->alivePersistentIds_.push_back(2);
    ssm_->alivePersistentIds_.push_back(3);
    SessionInfo info;
    info.bundleName_ = "IsWindowSupportCacheForRecovering";
    info.abilityName_ = "IsWindowSupportCacheForRecovering";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->isRecovered_ = true;
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession));
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    sceneSession1->isRecovered_ = false;
    ssm_->sceneSessionMap_.insert(std::make_pair(3, sceneSession1));
    sptr<SceneSession> sceneSession2 = nullptr;
    ssm_->sceneSessionMap_.insert(std::make_pair(4, sceneSession2));
    ssm_->ClearUnrecoveredSessions(recoveredPersistentIds);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    ssm_->recoveringFinished_ = true;
    auto ret = ssm_->IsWindowSupportCacheForRecovering(sceneSession, property);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: IsWindowSupportCacheForRecovering01
 * @tc.desc: test function : IsWindowSupportCacheForRecovering
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, IsWindowSupportCacheForRecovering01, Function | SmallTest | Level3)
{
    std::vector<int32_t> windowIds = {0, 1};
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert(std::make_pair(0, sceneSession));
    ssm_->OnNotifyAboveLockScreen(windowIds);

    SessionInfo info;
    info.bundleName_ = "IsWindowSupportCacheForRecovering01";
    info.abilityName_ = "IsWindowSupportCacheForRecovering01";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession1));
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    ssm_->recoveringFinished_ = false;
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    auto ret = ssm_->IsWindowSupportCacheForRecovering(sceneSession1, property);
    EXPECT_EQ(true, ret);
    property->SetWindowType(WindowType::APP_SUB_WINDOW_END);
    ret = ssm_->IsWindowSupportCacheForRecovering(sceneSession1, property);
    EXPECT_EQ(false, ret);
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ret = ssm_->IsWindowSupportCacheForRecovering(sceneSession1, property);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: IsWindowSupportCacheForRecovering02
 * @tc.desc: test function : IsWindowSupportCacheForRecovering
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, IsWindowSupportCacheForRecovering02, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.bundleName_ = "IsWindowSupportCacheForRecovering02";
    info.abilityName_ = "IsWindowSupportCacheForRecovering02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    ssm_->recoveringFinished_ = false;
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    property->parentPersistentId_ = 1;
    NotifyBindDialogSessionFunc func = [](const sptr<SceneSession>& sceneSession) {};
    ssm_->bindDialogTargetFuncMap_.insert(std::make_pair(1, func));
    auto ret = ssm_->IsWindowSupportCacheForRecovering(sceneSession, property);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: UnregisterSpecificSessionCreateListener
 * @tc.desc: test function : UnregisterSpecificSessionCreateListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, UnregisterSpecificSessionCreateListener, Function | SmallTest | Level3)
{
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->DestroyUIServiceExtensionSubWindow(sceneSession);
    ssm_->RegisterSessionInfoChangeNotifyManagerFunc(sceneSession);

    SessionInfo info;
    info.bundleName_ = "UnregisterSpecificSessionCreateListener";
    info.abilityName_ = "UnregisterSpecificSessionCreateListener";
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    ssm_->HandleHideNonSystemFloatingWindows(property, sceneSession);

    NotifyCreateKeyboardSessionFunc func = [](const sptr<SceneSession>& keyboardSession,
        const sptr<SceneSession>& panelSession) {};
    ssm_->SetCreateKeyboardSessionListener(func);

    ProcessOutsideDownEventFunc func1 = [](int32_t x, int32_t y) {};
    ssm_->outsideDownEventFunc_ = func1;
    ssm_->OnOutsideDownEvent(0, 0);

    ssm_->createSubSessionFuncMap_.clear();
    ssm_->bindDialogTargetFuncMap_.clear();
    NotifyBindDialogSessionFunc func2 = [](const sptr<SceneSession>& sceneSession) {};
    ssm_->bindDialogTargetFuncMap_.insert(std::make_pair(1, func2));
    ssm_->UnregisterSpecificSessionCreateListener(1);
    EXPECT_EQ(true, ssm_->bindDialogTargetFuncMap_.empty());
}

/**
 * @tc.name: GetIsLayoutFullScreen
 * @tc.desc: test function : GetIsLayoutFullScreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, GetIsLayoutFullScreen, Function | SmallTest | Level3)
{
    std::ostringstream oss;
    SessionInfo info;
    info.bundleName_ = "GetIsLayoutFullScreen";
    info.abilityName_ = "GetIsLayoutFullScreen";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->DumpSessionInfo(sceneSession, oss);

    ssm_->listenerController_ = std::make_shared<SessionListenerController>();
    ASSERT_NE(ssm_->listenerController_, nullptr);
    info.isSystem_ = true;
    ssm_->NotifyUnFocusedByMission(sceneSession);
    info.isSystem_ = false;
    ssm_->NotifyUnFocusedByMission(sceneSession);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    property->SetIsLayoutFullScreen(true);
    sceneSession->property_ = property;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    bool isLayoutFullScreen = true;
    auto ret = ssm_->GetIsLayoutFullScreen(isLayoutFullScreen);
    EXPECT_EQ(WSError::WS_OK, ret);
    property->SetIsLayoutFullScreen(false);
    ret = ssm_->GetIsLayoutFullScreen(isLayoutFullScreen);
    EXPECT_EQ(WSError::WS_OK, ret);
}
}
} // namespace Rosen
} // namespace OHOS