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
#include "mock/mock_accesstoken_kit.h"
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
    sptr<SceneSession> CreateSceneSession(const std::string& bundleName, WindowType windowType);
};

void SceneSessionManagerTest8::SetUpTestCase() {}

void SceneSessionManagerTest8::TearDownTestCase() {}

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

sptr<SceneSession> SceneSessionManagerTest8::CreateSceneSession(const std::string& bundleName, WindowType windowType)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = bundleName;
 
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(windowType);
    property->SetWindowName(bundleName);
 
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_ = property;
    return sceneSession;
}

namespace {

/**
 * @tc.name: GetRemoteSessionSnapshotInfo
 * @tc.desc: GetRemoteSessionSnapshotInfo set gesture navigation enabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, GetRemoteSessionSnapshotInfo, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, WindowLayerInfoChangeCallback, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, PostProcessFocus, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, PostProcessFocus01, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(0);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessFocus01";
    sessionInfo.abilityName_ = "PostProcessFocus01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->persistentId_ = 1;

    PostProcessFocusState state = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession->SetPostProcessFocusState(state);
    sceneSession->SetFocusableOnShow(false);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    ssm_->PostProcessFocus();
    EXPECT_EQ(0, focusGroup->GetFocusedSessionId());

    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->isVisible_ = true;
    ssm_->PostProcessFocus();
    EXPECT_NE(1, focusGroup->GetFocusedSessionId());
}

/**
 * @tc.name: PostProcessFocus03
 * @tc.desc: test function : PostProcessFocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, PostProcessFocus03, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessFocus03";
    sessionInfo.abilityName_ = "PostProcessFocus03";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->persistentId_ = 1;

    sceneSession->SetFocusedOnShow(false);
    PostProcessFocusState state = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession->SetPostProcessFocusState(state);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    ssm_->PostProcessFocus();
    EXPECT_EQ(sceneSession->IsFocusedOnShow(), false);

    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->isVisible_ = true;
    state = { true, true, true, FocusChangeReason::FOREGROUND };
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
HWTEST_F(SceneSessionManagerTest8, PostProcessProperty, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, NotifyUpdateRectAfterLayout, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, DestroyExtensionSession, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, FilterSceneSessionCovered, TestSize.Level1)
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
 * @tc.name: SubtractIntersectArea
 * @tc.desc: test function : SubtractIntersectArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, SubtractIntersectArea, TestSize.Level1)
{
    SkIRect rect{ .fLeft = 0, .fTop = 0, .fRight = 2880, .fBottom = 1920 };
    auto unaccountedSpace = std::make_shared<SkRegion>(rect);
    EXPECT_NE(unaccountedSpace, nullptr);

    sptr<SceneSession> sceneSession = nullptr;
    EXPECT_EQ(ssm_->SubtractIntersectArea(unaccountedSpace, sceneSession), false);

    SessionInfo sessionInfo;
    sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    WSRect wsRect{ .posX_ = 0, .posY_ = 0, .width_ = 100, .height_ = 100 };
    sceneSession->GetLayoutController()->SetSessionRect(wsRect);
    EXPECT_EQ(ssm_->SubtractIntersectArea(unaccountedSpace, sceneSession), true);

    SessionInfo sessionInfo2;
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    WSRect wsRect2 { .posX_ = 100, .posY_ = 150, .width_ = 100, .height_ = 100 };
    sceneSession2->GetLayoutController()->SetSessionRect(wsRect2);
    std::vector<Rect> hotAreas;
    hotAreas.push_back(Rect::EMPTY_RECT);
    hotAreas.push_back({.posX_ = 0, .posY_ = 0, .width_ = 10, .height_ = 10});
    sceneSession2->GetSessionProperty()->SetTouchHotAreas(hotAreas);
    EXPECT_EQ(ssm_->SubtractIntersectArea(unaccountedSpace, sceneSession2), true);
}

/**
 * @tc.name: UpdateSubWindowVisibility
 * @tc.desc: test function : UpdateSubWindowVisibility
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, UpdateSubWindowVisibility, TestSize.Level1)
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
    sceneSession2->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    struct RSSurfaceNodeConfig config;
    sceneSession2->surfaceNode_ = RSSurfaceNode::Create(config);
    ASSERT_NE(sceneSession2->surfaceNode_, nullptr);
    sceneSession2->surfaceNode_->id_ = 0;
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    currVisibleData.push_back(std::make_pair(0, WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION));
    ssm_->UpdateSubWindowVisibility(
        sceneSession, visibleState, visibilityChangeInfo, windowVisibilityInfos, visibilityInfo, currVisibleData);
}

/**
 * @tc.name: RegisterSessionChangeByActionNotifyManagerFunc
 * @tc.desc: test function : RegisterSessionChangeByActionNotifyManagerFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, RegisterSessionChangeByActionNotifyManagerFunc, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, RegisterSessionChangeByActionNotifyManagerFunc1, TestSize.Level1)
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

    sceneSession->NotifySessionChangeByActionNotifyManager(
        property, WSPropertyChangeAction::ACTION_UPDATE_NAVIGATION_INDICATOR_PROPS);

    sceneSession->NotifySessionChangeByActionNotifyManager(property,
                                                           WSPropertyChangeAction::ACTION_UPDATE_SET_BRIGHTNESS);

    sceneSession->NotifySessionChangeByActionNotifyManager(property,
                                                           WSPropertyChangeAction::ACTION_UPDATE_SYSTEM_PRIVACY_MODE);

    sceneSession->NotifySessionChangeByActionNotifyManager(property, WSPropertyChangeAction::ACTION_UPDATE_FLAGS);

    sceneSession->NotifySessionChangeByActionNotifyManager(property, WSPropertyChangeAction::ACTION_UPDATE_MODE);

    sceneSession->NotifySessionChangeByActionNotifyManager(
        property, WSPropertyChangeAction::ACTION_UPDATE_HIDE_NON_SYSTEM_FLOATING_WINDOWS);

    sceneSession->NotifySessionChangeByActionNotifyManager(property, WSPropertyChangeAction::ACTION_UPDATE_WINDOW_MASK);

    sceneSession->NotifySessionChangeByActionNotifyManager(property, WSPropertyChangeAction::ACTION_UPDATE_TOPMOST);
}

/**
 * @tc.name: RegisterRequestFocusStatusNotifyManagerFunc
 * @tc.desc: test function : RegisterRequestFocusStatusNotifyManagerFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, RegisterRequestFocusStatusNotifyManagerFunc, TestSize.Level1)
{
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->RegisterRequestFocusStatusNotifyManagerFunc(sceneSession);
    EXPECT_EQ(nullptr, sceneSession);
}

/**
 * @tc.name: HandleTurnScreenOn
 * @tc.desc: test function : HandleTurnScreenOn
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, HandleTurnScreenOn, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, HandleKeepScreenOn, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, SetBrightness, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, TerminateSessionNew, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, IsLastFrameLayoutFinished, TestSize.Level1)
{
    ssm_->closeTargetFloatWindowFunc_ = nullptr;
    std::string bundleName = "SetCloseTargetFloatWindowFunc";
    ProcessCloseTargetFloatWindowFunc func = [](const std::string& bundleName1) { return; };
    ssm_->SetCloseTargetFloatWindowFunc(func);

    IsRootSceneLastFrameLayoutFinishedFunc func1 = []() { return true; };
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
HWTEST_F(SceneSessionManagerTest8, ReportScreenFoldStatus, TestSize.Level1)
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

HWTEST_F(SceneSessionManagerTest8, GetWindowModeType, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSACalling(false);
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
HWTEST_F(SceneSessionManagerTest8, GetHostWindowRect, TestSize.Level1)
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
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(
        0, SuperFoldStatus::EXPANDED, { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    auto ret = ssm_->GetHostWindowRect(hostWindowId, rect);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_EQ(rect.posY_, 0);
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(
        0, SuperFoldStatus::KEYBOARD, { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    sceneSession->GetLayoutController()->SetSessionRect({ 0, 100, 0, 0 });
    ret = ssm_->GetHostWindowRect(hostWindowId, rect);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_EQ(rect.posY_, 100);

    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(
        0, SuperFoldStatus::HALF_FOLDED, { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1649, 2472, 40 });
    sceneSession->GetLayoutController()->SetSessionRect({ 0, 1000, 100, 100 });
    ret = ssm_->GetHostWindowRect(hostWindowId, rect);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_EQ(rect.posY_, 1000);
    sceneSession->GetLayoutController()->SetSessionRect({ 0, 2000, 100, 100 });
    ret = ssm_->GetHostWindowRect(hostWindowId, rect);
    WSRect hostRect = { 0, 2000, 100, 100 };
    sceneSession->TransformGlobalRectToRelativeRect(hostRect);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_EQ(rect.posY_, hostRect.posY_);

    sceneSession->GetSessionProperty()->SetIsSystemKeyboard(false);
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(
        0, SuperFoldStatus::UNKNOWN, { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    sceneSession->GetLayoutController()->SetSessionRect({ 0, 0, 0, 0 });
    ret = ssm_->GetHostWindowRect(hostWindowId, rect);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_EQ(rect.posY_, 0);
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(
        0, SuperFoldStatus::FOLDED, { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    sceneSession->GetLayoutController()->SetSessionRect({ 0, 100, 0, 0 });
    ret = ssm_->GetHostWindowRect(hostWindowId, rect);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_EQ(rect.posY_, 100);

    sceneSession->GetSessionProperty()->SetIsSystemKeyboard(true);
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(
        0, SuperFoldStatus::HALF_FOLDED, { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1649, 2472, 40 });
    sceneSession->GetLayoutController()->SetSessionRect({ 0, 1000, 100, 100 });
    ret = ssm_->GetHostWindowRect(hostWindowId, rect);
    EXPECT_EQ(WSError::WS_OK, ret);
    EXPECT_EQ(rect.posY_, 1000);
}

/**
 * @tc.name: GetHostGlobalScaledRect
 * @tc.desc: test function : GetHostGlobalScaledRect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, GetHostGlobalScaledRect, TestSize.Level1)
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
    info.bundleName_ = "GetHostGlobalScaledRect";
    info.abilityName_ = "GetHostGlobalScaledRect";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->sessionInfo_.screenId_ = 0;
    EXPECT_EQ(sceneSession->GetScreenId(), 0);
    ssm_->sceneSessionMap_.insert(std::make_pair(hostWindowId, sceneSession));
    auto ret = ssm_->GetHostGlobalScaledRect(hostWindowId, rect);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: NotifyStackEmpty
 * @tc.desc: test function : NotifyStackEmpty
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, NotifyStackEmpty, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, GetAppMainSceneSession, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, PostProcessProperty01, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, SetVmaCacheStatus, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, IsInDefaultScreen, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, OnSessionStateChange, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, OnSessionStateChange01, TestSize.Level1)
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
 * @tc.name: UnregisterSpecificSessionCreateListener
 * @tc.desc: test function : UnregisterSpecificSessionCreateListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, UnregisterSpecificSessionCreateListener, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest8, GetIsLayoutFullScreen, TestSize.Level1)
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

/**
 * @tc.name: RegisterWindowPropertyChangeAgent01
 * @tc.desc: test function : RegisterWindowPropertyChangeAgent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, RegisterWindowPropertyChangeAgent01, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSACalling(false);
    uint32_t interestInfo = 0;
    interestInfo |= static_cast<uint32_t>(WindowInfoKey::WINDOW_ID);
    sptr<IWindowManagerAgent> windowManagerAgent = nullptr;
    auto ret = ssm_->RegisterWindowPropertyChangeAgent(WindowInfoKey::DISPLAY_ID, interestInfo, windowManagerAgent);
    EXPECT_EQ(static_cast<uint32_t>(WindowInfoKey::DISPLAY_ID), ssm_->observedFlags_);
    EXPECT_EQ(static_cast<uint32_t>(WindowInfoKey::WINDOW_ID), ssm_->interestedFlags_);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);
    ssm_->observedFlags_ = 0;
    ssm_->interestedFlags_ = 0;
}

/**
 * @tc.name: UnregisterWindowPropertyChangeAgent01
 * @tc.desc: test function : UnregisterWindowPropertyChangeAgent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, UnregisterWindowPropertyChangeAgent01, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSACalling(false);
    uint32_t interestInfo = 0;
    interestInfo |= static_cast<uint32_t>(WindowInfoKey::WINDOW_ID);
    sptr<IWindowManagerAgent> windowManagerAgent = nullptr;
    auto ret = ssm_->RegisterWindowPropertyChangeAgent(WindowInfoKey::DISPLAY_ID, interestInfo, windowManagerAgent);
    ret = ssm_->UnregisterWindowPropertyChangeAgent(WindowInfoKey::DISPLAY_ID, interestInfo, windowManagerAgent);
    EXPECT_EQ(0, ssm_->observedFlags_);
    EXPECT_EQ(0, ssm_->interestedFlags_);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);
    ssm_->observedFlags_ = 0;
    ssm_->interestedFlags_ = 0;
}

/**
 * @tc.name: PackWindowPropertyChangeInfo01
 * @tc.desc: test function : PackWindowPropertyChangeInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, PackWindowPropertyChangeInfo01, TestSize.Level1)
{
    ssm_->interestedFlags_ = -1;
    SessionInfo sessionInfo1;
    sessionInfo1.isSystem_ = false;
    sessionInfo1.bundleName_ = "PackWindowPropertyChangeInfo";
    sessionInfo1.abilityName_ = "PackWindowPropertyChangeInfo";
    sessionInfo1.appIndex_ = 10;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    sceneSession1->SetVisibilityState(WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
    WSRect rect = { 0, 0, 100, 100 };
    sceneSession1->SetSessionRect(rect);
    sceneSession1->SetSessionGlobalRect(rect);
    sceneSession1->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession1->GetSessionProperty()->SetDisplayId(0);
    sceneSession1->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sceneSession1->SetFloatingScale(1.0f);
    sceneSession1->SetIsMidScene(true);

    std::unordered_map<WindowInfoKey, WindowChangeInfoType> windowPropertyChangeInfo;
    ssm_->PackWindowPropertyChangeInfo(sceneSession1, windowPropertyChangeInfo);
    EXPECT_EQ(windowPropertyChangeInfo.size(), 11);
}

/**
 * @tc.name: TestCheckSystemWindowPermission_Fb
 * @tc.desc: Test CheckSystemWindowPermission with windowType WINDOW_TYPE_FB then true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, TestCheckSystemWindowPermission_Fb, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    property->SetWindowType(WindowType::WINDOW_TYPE_FB);
    ASSERT_EQ(true, ssm_->CheckSystemWindowPermission(property));
}

/**
 * @tc.name: InitFbWindow
 * @tc.desc: test function : InitFbWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, InitFbWindow, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfo;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);

    ssm_->InitFbWindow(sceneSession, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    ssm_->InitFbWindow(sceneSession, property);

    property->SetWindowType(WindowType::WINDOW_TYPE_FB);
    ssm_->InitFbWindow(sceneSession, property);
    EXPECT_EQ(0, sceneSession->GetFbTemplateInfo().template_);
}

/**
@tc.name: GetFbPanelWindowId
@tc.desc: test function : GetFbPanelWindowId
@tc.type: FUNC
*/
HWTEST_F(SceneSessionManagerTest8, GetFbPanelWindowId, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    uint32_t windowId = 0;
    EXPECT_EQ(WMError::WM_ERROR_FB_INTERNAL_ERROR, ssm_->GetFbPanelWindowId(windowId));
    ssm_->sceneSessionMap_.insert({0, nullptr});
    ssm_->sceneSessionMap_.insert({1, CreateSceneSession("", WindowType::WINDOW_TYPE_PIP)});
    ssm_->sceneSessionMap_.insert({2, CreateSceneSession("SCBGlobalSearch7", WindowType::WINDOW_TYPE_FB)});
    sptr<SceneSession> sceneSession = CreateSceneSession("Fb_panel8", WindowType::WINDOW_TYPE_FB);
    ssm_->sceneSessionMap_.insert({3, sceneSession});

    MockAccesstokenKit::MockAccessTokenKitRet(0);
    EXPECT_EQ(WMError::WM_OK, ssm_->GetFbPanelWindowId(windowId));
    EXPECT_EQ(sceneSession->GetWindowId(), windowId);
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
}

/**
 * @tc.name: SetScreenPrivacyWindowTagSwitch01
 * @tc.desc: test function : SetScreenPrivacyWindowTagSwitch
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, SetScreenPrivacyWindowTagSwitch01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockIsSACalling(false);
    uint64_t screenId = 0;
    std::vector<std::string> privacyWindowTags = { "WMS_DEFAULT" };;
    bool enable = false;
    auto ret = ssm_->SetScreenPrivacyWindowTagSwitch(screenId, privacyWindowTags, enable);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);

    MockAccesstokenKit::MockIsSACalling(true);
    ret = ssm_->SetScreenPrivacyWindowTagSwitch(screenId, privacyWindowTags, enable);
    EXPECT_EQ(WMError::WM_OK, ret);

    enable = true;
    ret = ssm_->SetScreenPrivacyWindowTagSwitch(screenId, privacyWindowTags, enable);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: SetScreenPrivacyWindowTagSwitch02
 * @tc.desc: test function : SetScreenPrivacyWindowTagSwitch
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, SetScreenPrivacyWindowTagSwitch02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockIsSACalling(false);
    uint64_t screenId = 0;
    std::vector<std::string> privacyWindowTags;
    bool enable = false;
    auto ret = ssm_->SetScreenPrivacyWindowTagSwitch(screenId, privacyWindowTags, enable);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);

    MockAccesstokenKit::MockIsSACalling(true);
    ret = ssm_->SetScreenPrivacyWindowTagSwitch(screenId, privacyWindowTags, enable);
    EXPECT_EQ(WMError::WM_OK, ret);

    enable = true;
    ret = ssm_->SetScreenPrivacyWindowTagSwitch(screenId, privacyWindowTags, enable);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: AddSessionBlackList01
 * @tc.desc: test function : AddSessionBlackList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, AddSessionBlackList01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    MockAccesstokenKit::MockIsSACalling(false);
    std::unordered_set<std::string> bundleNames = { "test" };
    std::unordered_set<std::string> privacyWindowTags;
    auto ret = ssm_->AddSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);

    MockAccesstokenKit::MockIsSACalling(true);
    ret = ssm_->AddSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(WMError::WM_OK, ret);

    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "test";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    ssm_->sceneSessionMap_.insert({1, sceneSession1});
    ret = ssm_->AddSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: AddSessionBlackList02
 * @tc.desc: test function : AddSessionBlackList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, AddSessionBlackList02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    std::unordered_set<std::string> bundleNames = { "test" };
    std::unordered_set<std::string> privacyWindowTags = { "WMS_DEFAULT" };
    MockAccesstokenKit::MockIsSACalling(true);

    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "test";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    ssm_->sceneSessionMap_.insert({1, sceneSession1});
    auto ret = ssm_->AddSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: AddSessionBlackList03
 * @tc.desc: test function : AddSessionBlackList(sceneSession)
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, AddSessionBlackList03, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();

    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "test";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    std::vector<sptr<SceneSession>> sceneSessionList;
    sceneSessionList.emplace_back(sceneSession1);
    std::unordered_set<std::string> privacyWindowTags;
    auto ret = ssm_->AddSessionBlackList(sceneSessionList, privacyWindowTags);
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: AddSessionBlackList04
 * @tc.desc: test function : AddSessionBlackList(sceneSession)
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, AddSessionBlackList04, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();

    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "test";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    std::vector<sptr<SceneSession>> sceneSessionList;
    sceneSessionList.emplace_back(sceneSession1);
    std::unordered_set<std::string> privacyWindowTags;
    privacyWindowTags = { "WMS_DEFAULT" };
    auto ret = ssm_->AddSessionBlackList(sceneSessionList, privacyWindowTags);
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: RemoveSessionBlackList01
 * @tc.desc: test function : RemoveSessionBlackList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, RemoveSessionBlackList01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    MockAccesstokenKit::MockIsSACalling(false);
    std::unordered_set<std::string> bundleNames = { "test" };
    std::unordered_set<std::string> privacyWindowTags;
    auto ret = ssm_->RemoveSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);

    MockAccesstokenKit::MockIsSACalling(true);
    ret = ssm_->RemoveSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(WMError::WM_OK, ret);

    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "test";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    ssm_->sceneSessionMap_.insert({1, sceneSession1});
    std::vector<sptr<SceneSession>> sceneSessionList;
    sceneSessionList.emplace_back(sceneSession1);
    ssm_->AddSessionBlackList(sceneSessionList, privacyWindowTags);
    ret = ssm_->RemoveSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: RemoveSessionBlackList02
 * @tc.desc: test function : RemoveSessionBlackList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, RemoveSessionBlackList02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    std::unordered_set<std::string> bundleNames = { "test" };
    std::unordered_set<std::string> privacyWindowTags = { "WMS_DEFAULT" };
    MockAccesstokenKit::MockIsSACalling(true);

    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "test";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    ssm_->sceneSessionMap_.insert({1, sceneSession1});
    std::vector<sptr<SceneSession>> sceneSessionList;
    sceneSessionList.emplace_back(sceneSession1);
    ssm_->AddSessionBlackList(sceneSessionList, privacyWindowTags);
    auto ret = ssm_->RemoveSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: RemoveSessionBlackList03
 * @tc.desc: test function : RemoveSessionBlackList(sceneSession)
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, RemoveSessionBlackList03, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();

    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "test";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    std::vector<sptr<SceneSession>> sceneSessionList;
    sceneSessionList.emplace_back(sceneSession1);
    std::unordered_set<std::string> privacyWindowTags;
    ssm_->AddSessionBlackList(sceneSessionList, privacyWindowTags);
    auto ret = ssm_->RemoveSessionBlackList(sceneSessionList, privacyWindowTags);
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: RemoveSessionBlackList04
 * @tc.desc: test function : RemoveSessionBlackList(sceneSession)
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, RemoveSessionBlackList04, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();

    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "test";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    std::vector<sptr<SceneSession>> sceneSessionList;
    sceneSessionList.emplace_back(sceneSession1);
    std::unordered_set<std::string> privacyWindowTags = { "WMS_DEFAULT" };
    ssm_->AddSessionBlackList(sceneSessionList, privacyWindowTags);
    auto ret = ssm_->RemoveSessionBlackList(sceneSessionList, privacyWindowTags);
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: SessionBlackListSystemTest
 * @tc.desc: test function : SessionBlackListSystemTest
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, SessionBlackListSystemTest, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    std::size_t count = 100;
    for (std::size_t i = 0; i < count; ++i) {
        std::string key = "bundle_" + std::to_string(i);
        std::unordered_set<std::string> rsSet;
        rsSet.reserve(3);
        const std::string idx = std::to_string(i);
        rsSet.emplace("rs_" + idx + "_a");
        rsSet.emplace("rs_" + idx + "_b");
        rsSet.emplace("rs_" + idx + "_c");

        ssm_->sessionRSBlackListConfigSet_.insert({ .windowId = i, .privacyWindowTag = key });
        ssm_->bundleRSBlackListConfigMap_[key] = rsSet;
        for (std::size_t j = 0; j < count; ++j) {
            ssm_->sessionRSBlackListConfigSet_.insert({ j, "rs_" + idx + "_a" });
            ssm_->sessionRSBlackListConfigSet_.insert({ j, "rs_" + idx + "_b" });
            ssm_->sessionRSBlackListConfigSet_.insert({ j, "rs_" + idx + "_c" });
        }
        ssm_->sessionBlackListInfoMap_[i] = ssm_->sessionRSBlackListConfigSet_;

        ssm_->screenRSBlackListConfigMap_ [i].insert({ "rs_" + idx + "_a" });
        ssm_->screenRSBlackListConfigMap_ [i].insert({ "rs_" + idx + "_b" });
        ssm_->screenRSBlackListConfigMap_ [i].insert({ "rs_" + idx + "_c" });
    }

    for (std::size_t i = 0; i < count; ++i) {
        SessionInfo sessionInfo;
        sessionInfo.bundleName_ = "test";
        sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
        ssm_->sceneSessionMap_[i] = sceneSession;
    }
    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "test";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    std::vector<sptr<SceneSession>> sceneSessionList;
    sceneSessionList.emplace_back(sceneSession1);
    std::unordered_set<std::string> privacyWindowTags;
    std::vector<std::string> privacyWindowTagVactor;
    for (std::size_t i = 0; i < count; ++i) {
        const std::string idx = std::to_string(i);
        privacyWindowTags.insert("rs_" + idx + "_a");
        privacyWindowTags.insert("rs_" + idx + "_b");
        privacyWindowTags.insert("rs_" + idx + "_c");
        privacyWindowTagVactor.emplace_back("rs_" + idx + "_a");
        privacyWindowTagVactor.emplace_back("rs_" + idx + "_b");
        privacyWindowTagVactor.emplace_back("rs_" + idx + "_c");
    }

    auto ret = ssm_->SetScreenPrivacyWindowTagSwitch(1, privacyWindowTagVactor, true);
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = ssm_->SetScreenPrivacyWindowTagSwitch(1, privacyWindowTagVactor, false);
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = ssm_->RemoveSessionBlackList(sceneSessionList, privacyWindowTags);
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = ssm_->AddSessionBlackList(sceneSessionList, privacyWindowTags);
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->sceneSessionMap_.clear();
    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();
    ssm_->bundleRSBlackListConfigMap_.clear();
}

/**
 * @tc.name: FlushSessionBlackListInfoMapWhenAdd01
 * @tc.desc: test function : FlushSessionBlackListInfoMapWhenAdd()
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, FlushSessionBlackListInfoMapWhenAdd01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();

    SceneSessionManager::ScreenBlackListInfoSet info;
    info.insert({ .privacyWindowTag = "test" });
    ssm_->screenRSBlackListConfigMap_[0].insert(*info.begin());
    ssm_->sessionRSBlackListConfigSet_.insert({ .windowId = 0, .privacyWindowTag = "test" });

    auto ret = ssm_->FlushSessionBlackListInfoMapWhenAdd();
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();
}

/**
 * @tc.name: FlushSessionBlackListInfoMapWhenAdd02
 * @tc.desc: test function : FlushSessionBlackListInfoMapWhenAdd(screenId)
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, FlushSessionBlackListInfoMapWhenAdd02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();

    SceneSessionManager::ScreenBlackListInfoSet info;
    info.insert({ .privacyWindowTag = "test" });
    ssm_->screenRSBlackListConfigMap_[0].insert(*info.begin());
    ssm_->sessionRSBlackListConfigSet_.insert({ .windowId = 0, .privacyWindowTag = "test" });

    auto ret = ssm_->FlushSessionBlackListInfoMapWhenAdd();
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();
}

/**
 * @tc.name: FlushSessionBlackListInfoMapWhenRemove011
 * @tc.desc: test function : FlushSessionBlackListInfoMapWhenRemove()
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, FlushSessionBlackListInfoMapWhenRemove011, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();

    SceneSessionManager::ScreenBlackListInfoSet info;
    info.insert({ .privacyWindowTag = "test" });
    ssm_->screenRSBlackListConfigMap_[0].insert(*info.begin());
    ssm_->sessionRSBlackListConfigSet_.insert({ .windowId = 0, .privacyWindowTag = "test" });
    ssm_->sessionBlackListInfoMap_[0].insert({ .windowId = 0, .privacyWindowTag = "test1" });

    auto ret = ssm_->FlushSessionBlackListInfoMapWhenRemove();
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();
}

/**
 * @tc.name: FlushSessionBlackListInfoMapWhenRemove012
 * @tc.desc: test function : FlushSessionBlackListInfoMapWhenRemove()
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, FlushSessionBlackListInfoMapWhenRemove012, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();

    SceneSessionManager::ScreenBlackListInfoSet info;
    info.insert({ .privacyWindowTag = "test" });
    ssm_->screenRSBlackListConfigMap_[0].insert(*info.begin());
    ssm_->sessionBlackListInfoMap_[0].insert({ .windowId = 1, .privacyWindowTag = "WMS_DEFAULT" });

    auto ret = ssm_->FlushSessionBlackListInfoMapWhenRemove();
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();
}

/**
 * @tc.name: FlushSessionBlackListInfoMapWhenRemove021
 * @tc.desc: test function : FlushSessionBlackListInfoMapWhenRemove(screenId)
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, FlushSessionBlackListInfoMapWhenRemove021, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();

    SceneSessionManager::ScreenBlackListInfoSet info;
    info.insert({ .privacyWindowTag = "test" });
    ssm_->screenRSBlackListConfigMap_[0].insert(*info.begin());
    ssm_->sessionRSBlackListConfigSet_.insert({ .windowId = 0, .privacyWindowTag = "test" });
    ssm_->sessionBlackListInfoMap_[0].insert({ .windowId = 0, .privacyWindowTag = "test1" });

    auto ret = ssm_->FlushSessionBlackListInfoMapWhenRemove(0);
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();
}

/**
 * @tc.name: FlushSessionBlackListInfoMapWhenRemove022
 * @tc.desc: test function : FlushSessionBlackListInfoMapWhenRemove(screenId)
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, FlushSessionBlackListInfoMapWhenRemove022, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();

    SceneSessionManager::ScreenBlackListInfoSet info;
    info.insert({ .privacyWindowTag = "test" });
    ssm_->screenRSBlackListConfigMap_[0].insert(*info.begin());
    ssm_->sessionBlackListInfoMap_[0].insert({ .windowId = 1, .privacyWindowTag = "WMS_DEFAULT" });

    auto ret = ssm_->FlushSessionBlackListInfoMapWhenRemove(0);
    EXPECT_EQ(WMError::WM_OK, ret);

    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();
}

/**
 * @tc.name: AddskipSurfaceNodeIdSet
 * @tc.desc: test function : AddskipSurfaceNodeIdSet
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, AddskipSurfaceNodeIdSet01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    std::unordered_set<uint64_t> skipSurfaceNodeIdSet;

    ssm_->sceneSessionMap_.insert({1, nullptr });
    ssm_->AddskipSurfaceNodeIdSet(1, skipSurfaceNodeIdSet);
    EXPECT_EQ(skipSurfaceNodeIdSet.size(), 0);
    skipSurfaceNodeIdSet.clear();

    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "test";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    sceneSession1->GetSessionProperty()->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    ASSERT_NE(nullptr, surfaceNode);
    sceneSession1->SetSurfaceNode(surfaceNode);
    sceneSession1->GetSurfaceNode()->SetId(1001);
    sceneSession1->SetLeashWinSurfaceNode(surfaceNode);
    ssm_->sceneSessionMap_.insert({2, sceneSession1 });
    ssm_->AddskipSurfaceNodeIdSet(2, skipSurfaceNodeIdSet);
    EXPECT_EQ(skipSurfaceNodeIdSet.size(), 2);
    skipSurfaceNodeIdSet.clear();

    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: NotifyOnAttachToFrameNode01
 * @tc.desc: test function : NotifyOnAttachToFrameNode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, NotifyOnAttachToFrameNode01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();
    ssm_->bundleRSBlackListConfigMap_.clear();

    sptr<Session> session = nullptr;
    ssm_->NotifyOnAttachToFrameNode(session);
    EXPECT_EQ(ssm_->sessionBlackListInfoMap_.size(), 0);
    EXPECT_EQ(ssm_->sessionRSBlackListConfigSet_.size(), 0);

    SessionInfo info;
    session = sptr<Session>::MakeSptr(info);
    session->GetSessionProperty()->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ssm_->NotifyOnAttachToFrameNode(session);
    EXPECT_EQ(ssm_->sessionBlackListInfoMap_.size(), 0);
    EXPECT_EQ(ssm_->sessionRSBlackListConfigSet_.size(), 0);

    session->GetSessionProperty()->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    session->SetSurfaceNode(nullptr);
    ssm_->NotifyOnAttachToFrameNode(session);
    EXPECT_EQ(ssm_->sessionBlackListInfoMap_.size(), 0);
    EXPECT_EQ(ssm_->sessionRSBlackListConfigSet_.size(), 0);

    session->GetSessionProperty()->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    ASSERT_NE(nullptr, surfaceNode);
    session->SetSurfaceNode(surfaceNode);
    ssm_->NotifyOnAttachToFrameNode(session);
    EXPECT_EQ(ssm_->sessionBlackListInfoMap_.size(), 0);
    EXPECT_EQ(ssm_->sessionRSBlackListConfigSet_.size(), 0);

    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();
    ssm_->bundleRSBlackListConfigMap_.clear();
}

/**
 * @tc.name: SetSurfaceNodeIds01
 * @tc.desc: test function : SetSurfaceNodeIds
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, SetSurfaceNodeIds01, TestSize.Level1)
{
    ssm_->sessionBlackListInfoMap_.clear();
    ssm_->sceneSessionMap_.clear();
    std::vector<uint64_t> surfaceNodeIds;
    ssm_->sessionBlackListInfoMap_[0].insert({ .windowId = 0 });
    ssm_->sessionBlackListInfoMap_[0].insert({ .windowId = 0, .privacyWindowTag = "test" });
    ssm_->SetSurfaceNodeIds(0, surfaceNodeIds);
    EXPECT_EQ(ssm_->sessionBlackListInfoMap_[0].size(), 1);

    surfaceNodeIds.push_back(1);
    ssm_->SetSurfaceNodeIds(0, surfaceNodeIds);
    EXPECT_EQ(ssm_->sessionBlackListInfoMap_[0].size(), 1);

    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "test";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    ssm_->sceneSessionMap_.insert({1, sceneSession1});
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    ASSERT_NE(nullptr, surfaceNode);
    surfaceNode->SetId(1);
    sceneSession1->SetSurfaceNode(surfaceNode);
    ssm_->SetSurfaceNodeIds(0, surfaceNodeIds);
    EXPECT_EQ(ssm_->sessionBlackListInfoMap_[0].size(), 2);
}

/**
 * @tc.name: AddSkipSurfaceNodeWhenAttach01
 * @tc.desc: test function : AddSkipSurfaceNodeWhenAttach
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, AddSkipSurfaceNodeWhenAttach01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();
    ssm_->bundleRSBlackListConfigMap_.clear();

    int32_t persistentId = 1;
    ssm_->bundleRSBlackListConfigMap_["test"].insert({ "test" });
    ssm_->screenRSBlackListConfigMap_[0].insert({ .privacyWindowTag = "test" });

    ssm_->AddSkipSurfaceNodeWhenAttach(persistentId, "test", static_cast<uint64_t>(persistentId));
    EXPECT_EQ(ssm_->sessionBlackListInfoMap_.size(), 1);
    EXPECT_EQ(ssm_->sessionRSBlackListConfigSet_.size(), 1);

    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();
    ssm_->bundleRSBlackListConfigMap_.clear();
}

/**
 * @tc.name: NotifyBrightnessModeChange01
 * @tc.desc: test function : NotifyBrightnessModeChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, NotifyBrightnessModeChange01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);

    ssm_->SetDisplayBrightness(UNDEFINED_BRIGHTNESS);
    EXPECT_EQ(WMError::WM_DO_NOTHING, ssm_->NotifyBrightnessModeChange(""));
}

/**
 * @tc.name: NotifyBrightnessModeChange02
 * @tc.desc: test function : NotifyBrightnessModeChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, NotifyBrightnessModeChange02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);

    ssm_->SetDisplayBrightness(1);
    ssm_->brightnessSessionId_ = 1;

    ssm_->sceneSessionMap_.insert({1, nullptr});
    EXPECT_EQ(WMError::WM_DO_NOTHING, ssm_->NotifyBrightnessModeChange(""));

    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: NotifyBrightnessModeChange03
 * @tc.desc: test function : NotifyBrightnessModeChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, NotifyBrightnessModeChange03, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);

    ssm_->SetDisplayBrightness(1);
    ssm_->brightnessSessionId_ = 1;
    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "test";
    sessionInfo1.persistentId_ = 1;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    sceneSession1->state_ = SessionState::STATE_FOREGROUND;

    ssm_->sceneSessionMap_.insert({1, sceneSession1});
    EXPECT_EQ(WMError::WM_OK, ssm_->NotifyBrightnessModeChange(""));

    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: NotifyBrightnessModeChange04
 * @tc.desc: test function : NotifyBrightnessModeChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, NotifyBrightnessModeChange04, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);

    ssm_->SetDisplayBrightness(1);
    ssm_->brightnessSessionId_ = 1;
    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "test";
    sessionInfo1.persistentId_ = 1;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    sceneSession1->state_ = SessionState::STATE_DISCONNECT;

    ssm_->sceneSessionMap_.insert({1, sceneSession1});
    EXPECT_EQ(WMError::WM_DO_NOTHING, ssm_->NotifyBrightnessModeChange(""));

    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: RemoveSessionFromBlackListInfoSet
 * @tc.desc: test function : RemoveSessionFromBlackListInfoSet
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest8, RemoveSessionFromBlackListInfoSet01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();

    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "test";
    sessionInfo1.persistentId_ = 1;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    SceneSessionManager::SessionBlackListInfoSet sessionBlackListInfoSet;
    sessionBlackListInfoSet.insert({ .windowId = 0, .privacyWindowTag = "test" });

    ssm_->RemoveSessionFromBlackListInfoSet(sceneSession1, sessionBlackListInfoSet);
    EXPECT_EQ(sessionBlackListInfoSet.size(), 1);

    ssm_->screenRSBlackListConfigMap_.clear();
    ssm_->sessionRSBlackListConfigSet_.clear();
    ssm_->sessionBlackListInfoMap_.clear();
}
} // namespace
} // namespace Rosen
} // namespace OHOS