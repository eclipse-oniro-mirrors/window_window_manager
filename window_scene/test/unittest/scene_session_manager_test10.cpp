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

#include "interfaces/include/ws_common.h"
#include "iremote_object_mocker.h"
#include "mock/mock_accesstoken_kit.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/host/include/root_scene_session.h"
#include "session/host/include/scene_session.h"
#include "session_manager.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }
}
class SceneSessionManagerTest10 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void InitTestSceneSession(DisplayId displayId, int32_t windowId, int32_t zOrder, bool visible, WSRect rect);
    void InitTestSceneSessionForListWindowInfo();

    static sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerTest10::ssm_ = nullptr;

bool GetCutoutInfoByRotation(Rotation rotation, Rect& rect)
{
    auto cutoutInfo = DisplayManager::GetInstance().GetCutoutInfoWithRotation(rotation);
    if (cutoutInfo == nullptr) {
        TLOGI(WmsLogTag::WMS_IMMS, "There is no cutout info");
        return false;
    }
    std::vector<DMRect> cutoutAreas = cutoutInfo->GetBoundingRects();
    if (cutoutAreas.empty()) {
        TLOGI(WmsLogTag::WMS_IMMS, "There is no cutout area");
        return false;
    }
    for (auto& cutoutArea : cutoutAreas) {
        rect = { cutoutArea.posX_, cutoutArea.posY_, cutoutArea.width_, cutoutArea.height_ };
    }
    return true;
}

void SceneSessionManagerTest10::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerTest10::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerTest10::SetUp() {}

void SceneSessionManagerTest10::TearDown()
{
    MockAccesstokenKit::ChangeMockStateToInit();
    usleep(WAIT_SYNC_IN_NS);
}

void SceneSessionManagerTest10::InitTestSceneSession(DisplayId displayId,
                                                     int32_t windowId,
                                                     int32_t zOrder,
                                                     bool visible,
                                                     WSRect rect)
{
    SessionInfo info;
    info.bundleName_ = "root";
    info.persistentId_ = windowId;
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetZOrder(zOrder);
    sceneSession->SetRSVisible(visible);
    sceneSession->SetSessionRect(rect);
    sceneSession->property_->SetDisplayId(displayId);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    EXPECT_EQ(windowId, sceneSession->GetPersistentId());
}

void SceneSessionManagerTest10::InitTestSceneSessionForListWindowInfo()
{
    SessionInfo sessionInfo1;
    sessionInfo1.isSystem_ = false;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    sceneSession1->SetVisibilityState(WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
    WSRect rect = { 0, 0, 100, 100 };
    sceneSession1->SetSessionRect(rect);
    sceneSession1->SetSessionGlobalRect(rect);
    sceneSession1->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession1->GetSessionProperty()->SetDisplayId(0);
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });

    SessionInfo sessionInfo2;
    sessionInfo2.isSystem_ = false;
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    sceneSession2->SetVisibilityState(WINDOW_VISIBILITY_STATE_PARTICALLY_OCCLUSION);
    rect = { 0, 0, 120, 120 };
    sceneSession2->SetSessionRect(rect);
    sceneSession2->SetSessionGlobalRect(rect);
    sceneSession2->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession2->GetSessionProperty()->SetDisplayId(0);
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });

    SessionInfo sessionInfo3;
    sessionInfo3.isSystem_ = false;
    sptr<SceneSession> sceneSession3 = sptr<SceneSession>::MakeSptr(sessionInfo3, nullptr);
    sceneSession3->SetVisibilityState(WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    rect = { 0, 100, 120, 120 };
    sceneSession3->SetSessionRect(rect);
    sceneSession3->SetSessionGlobalRect(rect);
    sceneSession3->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession3->GetSessionProperty()->SetDisplayId(0);
    ssm_->sceneSessionMap_.insert({ sceneSession3->GetPersistentId(), sceneSession3 });

    SessionInfo sessionInfo4;
    sessionInfo4.isSystem_ = false;
    sptr<SceneSession> sceneSession4 = sptr<SceneSession>::MakeSptr(sessionInfo4, nullptr);
    sceneSession4->SetVisibilityState(WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    rect = { 0, 3000, 120, 120 };
    sceneSession4->SetSessionRect(rect);
    sceneSession4->SetSessionGlobalRect(rect);
    sceneSession4->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession4->GetSessionProperty()->SetDisplayId(0);
    ssm_->sceneSessionMap_.insert({ sceneSession4->GetPersistentId(), sceneSession4 });

    SessionInfo sessionInfo5;
    sessionInfo5.isSystem_ = false;
    sptr<SceneSession> sceneSession5 = sptr<SceneSession>::MakeSptr(sessionInfo5, nullptr);
    sceneSession5->SetVisibilityState(WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
    rect = { 0, 0, 100, 100 };
    sceneSession5->SetSessionRect(rect);
    sceneSession5->SetSessionGlobalRect(rect);
    sceneSession5->SetSessionState(SessionState::STATE_BACKGROUND);
    sceneSession5->GetSessionProperty()->SetDisplayId(0);
    ssm_->sceneSessionMap_.insert({ sceneSession5->GetPersistentId(), sceneSession5 });

    SessionInfo sessionInfo6;
    sessionInfo6.isSystem_ = true;
    sptr<SceneSession> sceneSession6 = sptr<SceneSession>::MakeSptr(sessionInfo6, nullptr);
    sceneSession6->SetVisibilityState(WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    rect = { 0, 200, 120, 120 };
    sceneSession6->SetSessionRect(rect);
    sceneSession6->SetSessionGlobalRect(rect);
    sceneSession6->SetSessionState(SessionState::STATE_FOREGROUND);
    constexpr DisplayId SECOND_DISPLAY_ID = 11;
    sceneSession6->GetSessionProperty()->SetDisplayId(SECOND_DISPLAY_ID);
    ssm_->sceneSessionMap_.insert({ sceneSession6->GetPersistentId(), sceneSession6 });
}

namespace {
/**
 * @tc.name: RequestSceneSessionDestructionInner
 * @tc.desc: Test RequestSceneSessionDestructionInner with CollaboratorType RESERVE_TYPE
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, RequestSceneSessionDestructionInner, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);

    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback = nullptr;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    sptr<AAFwk::SessionInfo> sceneSessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    bool needRemoveSession = true;
    bool isForceClean = true;

    SessionInfo sessionInfo;
    sessionInfo.collaboratorType_ = CollaboratorType::RESERVE_TYPE;
    auto res =
        ssm_->RequestSceneSessionDestructionInner(sceneSession, sceneSessionInfo, needRemoveSession, isForceClean);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: TestRequestSceneSessionDestructionInner_01
 * @tc.desc: Test RequestSceneSessionDestructionInner with CollaboratorType DEFAULT_TYPE
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestRequestSceneSessionDestructionInner_01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);

    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> specificCallback = nullptr;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCallback);
    sptr<AAFwk::SessionInfo> sceneSessionInfo = sptr<AAFwk::SessionInfo>::MakeSptr();
    bool needRemoveSession = false;
    bool isForceClean = false;

    SessionInfo sessionInfo;
    sessionInfo.collaboratorType_ = CollaboratorType::DEFAULT_TYPE;
    sessionInfo.want = std::make_shared<AAFwk::Want>();
    ssm_->listenerController_ = std::make_shared<SessionListenerController>();
    auto res =
        ssm_->RequestSceneSessionDestructionInner(sceneSession, sceneSessionInfo, needRemoveSession, isForceClean);
    ASSERT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: TestRegisterWindowManagerAgent_01
 * @tc.desc: Test RegisterWindowManagerAgent with WindowManagerAgentType WINDOW_MANAGER_AGENT_TYPE_SYSTEM_BAR
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestRegisterWindowManagerAgent_01, TestSize.Level1)
{
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_SYSTEM_BAR;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto res = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: TestRegisterWindowManagerAgent_02
 * @tc.desc: Test RegisterWindowManagerAgent with AgentType WINDOW_MANAGER_AGENT_TYPE_GESTURE_NAVIGATION_ENABLED
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestRegisterWindowManagerAgent_02, TestSize.Level1)
{
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GESTURE_NAVIGATION_ENABLED;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto res = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: TestRegisterWindowManagerAgent_03
 * @tc.desc: Test RegisterWindowManagerAgent with WindowManagerAgentType WINDOW_MANAGER_AGENT_TYPE_WATER_MARK_FLAG
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestRegisterWindowManagerAgent_03, TestSize.Level1)
{
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WATER_MARK_FLAG;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto res = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: TestRegisterWindowManagerAgent_04
 * @tc.desc: Test RegisterWindowManagerAgent with WindowManagerAgentType WINDOW_MANAGER_AGENT_TYPE_WINDOW_UPDATE
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestRegisterWindowManagerAgent_04, TestSize.Level1)
{
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_UPDATE;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto res = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    ASSERT_EQ(res, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: TestRegisterWindowManagerAgent_05
 * @tc.desc: Test RegisterWindowManagerAgent with WindowManagerAgentType WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestRegisterWindowManagerAgent_05, TestSize.Level1)
{
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto res = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: TestRegisterWindowManagerAgent_06
 * @tc.desc: Test RegisterWindowManagerAgent with WindowManagerAgentType WINDOW_MANAGER_AGENT_TYPE_FOCUS
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestRegisterWindowManagerAgent_06, TestSize.Level1)
{
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto res = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: TestRegisterWindowManagerAgent_07
 * @tc.desc: Test RegisterWindowManagerAgent with WindowManagerAgentType WINDOW_MANAGER_AGENT_TYPE_WINDOW_DRAWING_STATE
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestRegisterWindowManagerAgent_07, TestSize.Level1)
{
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_DRAWING_STATE;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto res = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: TestRegisterWindowManagerAgent_08
 * @tc.desc: Test RegisterWindowManagerAgent with WindowManagerAgentType WINDOW_MANAGER_AGENT_TYPE_VISIBLE_WINDOW_NUM
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestRegisterWindowManagerAgent_08, TestSize.Level1)
{
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_VISIBLE_WINDOW_NUM;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto res = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: TestRegisterWindowManagerAgent_09
 * @tc.desc: Test RegisterWindowManagerAgent with WindowManagerAgentType WINDOW_MANAGER_AGENT_TYPE_WINDOW_MODE
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestRegisterWindowManagerAgent_09, TestSize.Level1)
{
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_MODE;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto res = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: TestRegisterWindowManagerAgent_010
 * @tc.desc: Test RegisterWindowManagerAgent with WindowManagerAgentType WINDOW_MANAGER_AGENT_TYPE_WINDOW_PID_VISIBILITY
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestRegisterWindowManagerAgent_010, TestSize.Level1)
{
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_PID_VISIBILITY;
    sptr<IWindowManagerAgent> windowManagerAgent;
    auto res = ssm_->RegisterWindowManagerAgent(type, windowManagerAgent);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: RegisterAcquireRotateAnimationConfigFunc
 * @tc.desc: RegisterAcquireRotateAnimationConfigFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, RegisterAcquireRotateAnimationConfigFunc, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "bundleName";
    sessionInfo.persistentId_ = 1;
    sessionInfo.isSystem_ = false;
    sessionInfo.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    ASSERT_NE(sessionInfo.abilityInfo, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->scenePersistence_ = sptr<ScenePersistence>::MakeSptr("bundleName", 1);
    ASSERT_NE(sceneSession->scenePersistence_, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->RegisterAcquireRotateAnimationConfigFunc(sceneSession);
    WSRect rect({ 1, 1, 1, 1 });
    SizeChangeReason reason = SizeChangeReason::ROTATION;
    WSError result = sceneSession->UpdateRect(rect, reason, "SceneSessionManagerTest10");
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: TestCheckLastFocusedAppSessionFocus_01
 * @tc.desc: Test CheckLastFocusedAppSessionFocus with GetPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestCheckLastFocusedAppSessionFocus_01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);

    SessionInfo info1;
    info1.abilityName_ = "focusedSession";
    info1.bundleName_ = "focusedSession";
    info1.windowType_ = 1;
    sptr<SceneSession> focusedSession = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(focusedSession, nullptr);

    SessionInfo info2;
    info2.abilityName_ = "nextSession";
    info2.bundleName_ = "nextSession";
    info2.windowType_ = 1;
    sptr<SceneSession> nextSession = sptr<SceneSession>::MakeSptr(info2, nullptr);
    ASSERT_NE(nextSession, nullptr);

    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetLastFocusedAppSessionId(nextSession->GetPersistentId());
    ASSERT_EQ(false, ssm_->CheckLastFocusedAppSessionFocus(focusedSession, nextSession));
}

/**
 * @tc.name: TestCheckLastFocusedAppSessionFocus_02
 * @tc.desc: Test CheckLastFocusedAppSessionFocus with WindowType WINDOW_TYPE_DIALOG
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestCheckLastFocusedAppSessionFocus_02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);

    SessionInfo info1;
    info1.abilityName_ = "focusedSession";
    info1.bundleName_ = "focusedSession";
    info1.windowType_ = 1;
    sptr<SceneSession> focusedSession = sptr<SceneSession>::MakeSptr(info1, nullptr);

    SessionInfo info2;
    info2.abilityName_ = "nextSession";
    info2.bundleName_ = "nextSession";
    info2.windowType_ = 1;
    sptr<SceneSession> nextSession = sptr<SceneSession>::MakeSptr(info2, nullptr);

    ssm_->windowFocusController_->UpdateFocusedAppSessionId(DEFAULT_DISPLAY_ID, 124);
    focusedSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ASSERT_EQ(false, ssm_->CheckLastFocusedAppSessionFocus(focusedSession, nextSession));

    nextSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    ASSERT_EQ(false, ssm_->CheckLastFocusedAppSessionFocus(focusedSession, nextSession));
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    ASSERT_EQ(0, focusGroup->GetLastFocusedAppSessionId());
}

/**
 * @tc.name: GetWindowIdsByCoordinate01
 * @tc.desc: GetWindowIdsByCoordinate, displayId invalid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, GetWindowIdsByCoordinate01, TestSize.Level1)
{
    std::vector<int32_t> windowIds;
    WMError result = ssm_->GetWindowIdsByCoordinate(DISPLAY_ID_INVALID, 0, 0, 0, windowIds);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: GetWindowIdsByCoordinate02
 * @tc.desc: GetWindowIdsByCoordinate, windowNumber 0, x y invalid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, GetWindowIdsByCoordinate02, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    InitTestSceneSession(1, 101, 11, true, { 100, 100, 200, 200 });
    ssm_->sceneSessionMap_.insert({ 102, nullptr });
    InitTestSceneSession(1, 103, 14, true, { 120, 120, 220, 220 });
    InitTestSceneSession(1, 105, 12, true, { 100, 100, 200, 200 });
    auto it1 = ssm_->sceneSessionMap_.find(105);
    if (it1 != ssm_->sceneSessionMap_.end()) {
        it1->second->sessionInfo_.bundleName_ = "other";
    }
    InitTestSceneSession(1, 106, 15, true, { 140, 140, 240, 240 });
    InitTestSceneSession(2, 107, 15, true, { 150, 150, 250, 250 });
    InitTestSceneSession(1, 108, 13, false, { 150, 150, 250, 250 });
    InitTestSceneSession(1, 109, 13, true, { 160, 160, 260, 260 });
    InitTestSceneSession(1, 110, 12, true, { 500, 500, 600, 600 });

    std::vector<int32_t> windowIds;
    WMError result = ssm_->GetWindowIdsByCoordinate(1, 0, -1, -1, windowIds);
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(5, windowIds.size());
    EXPECT_EQ(106, windowIds[0]);
    EXPECT_EQ(103, windowIds[1]);
    EXPECT_EQ(109, windowIds[2]);
    EXPECT_EQ(110, windowIds[3]);
    EXPECT_EQ(101, windowIds[4]);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: GetWindowIdsByCoordinate03
 * @tc.desc: GetWindowIdsByCoordinate, windowNumber 3, x y invalid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, GetWindowIdsByCoordinate03, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    InitTestSceneSession(1, 111, 11, true, { 100, 100, 200, 200 });
    ssm_->sceneSessionMap_.insert({ 102, nullptr });
    InitTestSceneSession(1, 113, 14, true, { 120, 120, 220, 220 });
    InitTestSceneSession(1, 114, 12, true, { 100, 100, 200, 200 });
    ASSERT_TRUE(ssm_->sceneSessionMap_.find(114) != ssm_->sceneSessionMap_.end());
    InitTestSceneSession(1, 115, 12, true, { 100, 100, 200, 200 });
    auto it1 = ssm_->sceneSessionMap_.find(115);
    if (it1 != ssm_->sceneSessionMap_.end()) {
        it1->second->sessionInfo_.bundleName_ = "other";
    }
    InitTestSceneSession(1, 116, 15, true, { 140, 140, 240, 240 });
    InitTestSceneSession(2, 117, 15, true, { 150, 150, 250, 250 });
    InitTestSceneSession(1, 118, 13, false, { 150, 150, 250, 250 });
    InitTestSceneSession(1, 119, 13, true, { 160, 160, 260, 260 });
    InitTestSceneSession(1, 120, 12, true, { 500, 500, 600, 600 });

    std::vector<int32_t> windowIds;
    WMError result = ssm_->GetWindowIdsByCoordinate(1, 3, -1, -1, windowIds);
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(3, windowIds.size());
    EXPECT_EQ(116, windowIds[0]);
    EXPECT_EQ(113, windowIds[1]);
    EXPECT_EQ(119, windowIds[2]);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: GetWindowIdsByCoordinate04
 * @tc.desc: GetWindowIdsByCoordinate, windowNumber 0, x y effictive value
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, GetWindowIdsByCoordinate04, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    InitTestSceneSession(1, 121, 11, true, { 100, 100, 200, 200 });
    ssm_->sceneSessionMap_.insert({ 102, nullptr });
    InitTestSceneSession(1, 123, 14, true, { 120, 120, 220, 220 });
    InitTestSceneSession(1, 125, 12, true, { 100, 100, 200, 200 });
    auto it1 = ssm_->sceneSessionMap_.find(125);
    if (it1 != ssm_->sceneSessionMap_.end()) {
        it1->second->sessionInfo_.bundleName_ = "other";
    }
    InitTestSceneSession(1, 126, 15, true, { 140, 140, 240, 240 });
    InitTestSceneSession(2, 127, 15, true, { 150, 150, 250, 250 });
    InitTestSceneSession(1, 128, 13, false, { 150, 150, 250, 250 });
    InitTestSceneSession(1, 129, 13, true, { 160, 160, 260, 260 });
    InitTestSceneSession(1, 130, 12, true, { 500, 500, 600, 600 });

    std::vector<int32_t> windowIds;
    WMError result = ssm_->GetWindowIdsByCoordinate(1, 0, 180, 180, windowIds);
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(4, windowIds.size());
    EXPECT_EQ(126, windowIds[0]);
    EXPECT_EQ(123, windowIds[1]);
    EXPECT_EQ(129, windowIds[2]);
    EXPECT_EQ(121, windowIds[3]);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: GetWindowIdsByCoordinate05
 * @tc.desc: GetWindowIdsByCoordinate, windowNumber 3, x y effictive value
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, GetWindowIdsByCoordinate05, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    InitTestSceneSession(1, 131, 11, true, { 100, 100, 200, 200 });
    ssm_->sceneSessionMap_.insert({ 102, nullptr });
    InitTestSceneSession(1, 133, 14, true, { 120, 120, 220, 220 });
    InitTestSceneSession(1, 134, 12, true, { 100, 100, 200, 200 });
    ASSERT_TRUE(ssm_->sceneSessionMap_.find(134) != ssm_->sceneSessionMap_.end());
    InitTestSceneSession(1, 135, 12, true, { 100, 100, 200, 200 });
    auto it1 = ssm_->sceneSessionMap_.find(135);
    if (it1 != ssm_->sceneSessionMap_.end()) {
        it1->second->sessionInfo_.bundleName_ = "other";
    }
    InitTestSceneSession(1, 136, 15, true, { 140, 140, 240, 240 });
    InitTestSceneSession(2, 137, 15, true, { 150, 150, 250, 250 });
    InitTestSceneSession(1, 138, 13, false, { 150, 150, 250, 250 });
    InitTestSceneSession(1, 139, 13, true, { 160, 160, 260, 260 });
    InitTestSceneSession(1, 140, 12, true, { 500, 500, 600, 600 });

    std::vector<int32_t> windowIds;
    WMError result = ssm_->GetWindowIdsByCoordinate(1, 3, 180, 180, windowIds);
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(3, windowIds.size());
    EXPECT_EQ(136, windowIds[0]);
    EXPECT_EQ(133, windowIds[1]);
    EXPECT_EQ(139, windowIds[2]);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: ChangeWindowRectYInVirtualDisplay
 * @tc.desc: ChangeWindowRectYInVirtualDisplay
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, ChangeWindowRectYInVirtualDisplay, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    DisplayId defaultDisplayId = 0;
    DisplayId displayId = 0;
    int32_t y = 100;
    ssm_->ChangeWindowRectYInVirtualDisplay(displayId, y);
    EXPECT_TRUE(g_errLog.find("This is not VIRTUAL_DISPLAY_ID") != std::string::npos);
    EXPECT_EQ(y, 100);
    displayId = 999;
    ssm_->ChangeWindowRectYInVirtualDisplay(displayId, y);
    EXPECT_EQ(displayId, defaultDisplayId);
    LOG_SetCallback(nullptr);
}
/**
 * @tc.name: ProcessFocusZOrderChange
 * @tc.desc: ProcessFocusZOrderChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, ProcessFocusZOrderChange, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    ssm_->ProcessFocusZOrderChange(10);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ssm_->ProcessFocusZOrderChange(97);

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ssm_->ProcessFocusZOrderChange(97);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "focusedSession";
    sessionInfo.abilityName_ = "focusedSession";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    ssm_->ProcessFocusZOrderChange(97);

    sceneSession->lastZOrder_ = 2203;
    sceneSession->zOrder_ = 101;
    ssm_->ProcessFocusZOrderChange(97);

    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "voiceInteractionSession";
    sessionInfo1.abilityName_ = "voiceInteractionSession";
    sessionInfo1.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_VOICE_INTERACTION);
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    sceneSession1->zOrder_ = 2109;
    ssm_->sceneSessionMap_.emplace(2, sceneSession1);

    sceneSession->lastZOrder_ = 103;
    sceneSession->zOrder_ = 101;
    ssm_->ProcessFocusZOrderChange(97);

    sceneSession->lastZOrder_ = 2203;
    sceneSession->zOrder_ = 101;
    ssm_->ProcessFocusZOrderChange(97);
}

/**
 * @tc.name: GetAllSceneSessionForAccessibility
 * @tc.desc: GetAllSceneSessionForAccessibility
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, GetAllSceneSessionForAccessibility, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    std::vector<sptr<SceneSession>> sceneSessionList;
    ASSERT_EQ(sceneSessionList.size(), 0);
    SessionInfo info1;
    info1.abilityName_ = "test1";
    info1.bundleName_ = "test1";
    info1.windowType_ = 1;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    SessionInfo info2;
    info2.abilityName_ = "test2";
    info2.bundleName_ = "test2";
    info2.windowType_ = 1;
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    ASSERT_NE(nullptr, sceneSession2);
    SessionInfo info3;
    info3.abilityName_ = "test3";
    info3.bundleName_ = "test3";
    info3.windowType_ = 1;
    sptr<SceneSession> sceneSession3 = sptr<SceneSession>::MakeSptr(info3, nullptr);
    ASSERT_NE(nullptr, sceneSession3);
    ssm_->sceneSessionMap_.emplace(1, sceneSession1);
    ssm_->sceneSessionMap_.emplace(2, sceneSession2);
    ssm_->sceneSessionMap_.emplace(3, sceneSession3);
    sceneSession1->isScbCoreEnabled_ = false;
    sceneSession2->isScbCoreEnabled_ = false;
    sceneSession3->isScbCoreEnabled_ = false;
    sceneSession1->isVisibleForAccessibility_ = true;
    sceneSession2->isVisibleForAccessibility_ = false;
    sceneSession3->isVisibleForAccessibility_ = true;
    sceneSession1->isVisible_ = true;
    sceneSession2->isVisible_ = false;
    sceneSession3->isVisible_ = true;
    ssm_->GetAllSceneSessionForAccessibility(sceneSessionList);
    ASSERT_EQ(sceneSessionList.size(), 1);
}

/**
 * @tc.name: TestGetMainParentSceneSession_01
 * @tc.desc: Test GetMainParentSceneSession with invalid persistentId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestGetMainParentSceneSession_01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<SceneSession> ret = ssm_->GetMainParentSceneSession(0, ssm_->sceneSessionMap_);
    ASSERT_EQ(ret, nullptr);
}

/**
 * @tc.name: TestGetMainParentSceneSession_02
 * @tc.desc: Test GetMainParentSceneSession with Error found scene session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestGetMainParentSceneSession_02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<SceneSession> ret = ssm_->GetMainParentSceneSession(999, ssm_->sceneSessionMap_);
    ASSERT_EQ(ret, nullptr);
}

/**
 * @tc.name: TestGetMainParentSceneSession_03
 * @tc.desc: Test GetMainParentSceneSession with parent session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestGetMainParentSceneSession_03, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "GetMainParentSceneSession001";
    info.bundleName_ = "GetMainParentSceneSession001";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSessionParent = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSessionParent->property_->SetPersistentId(100);
    ssm_->sceneSessionMap_.insert({ 100, sceneSessionParent });
    sceneSession->SetParentSession(sceneSessionParent);

    ssm_->sceneSessionMap_[999] = sceneSession;
    sptr<SceneSession> ret = ssm_->GetMainParentSceneSession(999, ssm_->sceneSessionMap_);
    ASSERT_NE(ret, sceneSessionParent);
}

/**
 * @tc.name: TestGetParentMainWindowId_01
 * @tc.desc: Tst GetParentMainWindowId with invalid windowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestGetParentMainWindowId_01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    int32_t windowId = 0;
    int32_t mainWindowId = 0;
    WMError ret = ssm_->GetParentMainWindowId(windowId, mainWindowId);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: TestGetParentMainWindowId_02
 * @tc.desc: Tst GetParentMainWindowId with not found scene session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestGetParentMainWindowId_02, TestSize.Level1)
{
    int32_t windowId = -1;
    int32_t mainWindowId = 0;
    WMError ret = ssm_->GetParentMainWindowId(windowId, mainWindowId);
    ASSERT_EQ(ret, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: TestGetParentMainWindowId_03
 * @tc.desc: Tst GetParentMainWindowId with session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestGetParentMainWindowId_03, TestSize.Level1)
{
    int32_t windowId = 200;
    int32_t mainWindowId = 0;
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->property_->SetPersistentId(windowId);
    ssm_->sceneSessionMap_[windowId] = sceneSession;
    WMError ret = ssm_->GetParentMainWindowId(windowId, mainWindowId);
    ASSERT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: TestGetParentMainWindowId_04
 * @tc.desc: Tst GetParentMainWindowId with WindowType WINDOW_TYPE_APP_SUB_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestGetParentMainWindowId_04, TestSize.Level1)
{
    int32_t windowId = 200;
    int32_t mainWindowId = 0;
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->property_->SetPersistentId(windowId);
    ssm_->sceneSessionMap_[windowId] = sceneSession;
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    WMError ret = ssm_->GetParentMainWindowId(windowId, mainWindowId);
    ASSERT_EQ(ret, WMError::WM_ERROR_NULLPTR);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ret = ssm_->GetParentMainWindowId(windowId, mainWindowId);
    ASSERT_EQ(ret, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: NotifyVisibleChange
 * @tc.desc: test NotifyVisibleChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, NotifyVisibleChange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);

    ASSERT_FALSE(ssm_->NotifyVisibleChange(sceneSession->GetPersistentId()));
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ASSERT_TRUE(ssm_->NotifyVisibleChange(sceneSession->GetPersistentId()));

    ssm_->sceneSessionMap_.erase(sceneSession->GetPersistentId());
}

/**
 * @tc.name: TestIsInDefaultScreen_01
 * @tc.desc: Test IsInDefaultScreen with not DefaultScreen id
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestIsInDefaultScreen_01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    DisplayId displayId = 5;
    sceneSession->property_->SetDisplayId(displayId);
    ASSERT_EQ(ssm_->IsInDefaultScreen(sceneSession), false);
}

/**
 * @tc.name: TestIsInDefaultScreen_02
 * @tc.desc: Test IsInDefaultScreen with DefaultScreen id
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestIsInDefaultScreen_02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    DisplayId displayId = ScreenSessionManagerClient::GetInstance().GetDefaultScreenId();
    sceneSession->property_->SetDisplayId(displayId);
    ASSERT_EQ(ssm_->IsInDefaultScreen(sceneSession), true);
}

/**
 * @tc.name: RegisterSessionPropertyChangeNotifyManagerFunc
 * @tc.desc: test RegisterSessionPropertyChangeNotifyManagerFunc01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, RegisterSessionPropertyChangeNotifyManagerFunc01, TestSize.Level1)
{
    ssm_->RegisterSessionPropertyChangeNotifyManagerFunc(nullptr);
    SessionInfo info;
    info.abilityName_ = "RegisterRequestVsyncFunc01";
    info.bundleName_ = "RegisterRequestVsyncFunc01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->RegisterSessionPropertyChangeNotifyManagerFunc(sceneSession);
    EXPECT_NE(nullptr, sceneSession->sessionPropertyChangeNotifyManagerFunc_);
}

/**
 * @tc.name: TestEraseSceneSessionAndMarkDirtyLocked_01
 * @tc.desc: Test EraseSceneSessionAndMarkDirtyLocked with erase id not exist
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestEraseSceneSessionAndMarkDirtyLocked_01, TestSize.Level1)
{
    // init
    ssm_->sceneSessionMap_.clear();
    ssm_->sessionMapDirty_ = 0;

    SessionInfo info;
    info.abilityName_ = "EraseSceneSessionAndMarkDirtyLocked";
    info.bundleName_ = "EraseSceneSessionAndMarkDirtyLocked";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    const int32_t validId = 100;
    const int32_t invalidId = 101;
    ssm_->sceneSessionMap_.insert({ validId, sceneSession });
    // erase id not exist
    ssm_->EraseSceneSessionAndMarkDirtyLocked(invalidId);
    ASSERT_EQ(ssm_->sessionMapDirty_, 0);
    ASSERT_NE(ssm_->sceneSessionMap_.find(validId), ssm_->sceneSessionMap_.end());
}

/**
 * @tc.name: TestEraseSceneSessionAndMarkDirtyLocked_02
 * @tc.desc: Test EraseSceneSessionAndMarkDirtyLocked with erase invisible session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestEraseSceneSessionAndMarkDirtyLocked_02, TestSize.Level1)
{
    // init
    ssm_->sceneSessionMap_.clear();
    ssm_->sessionMapDirty_ = 0;

    SessionInfo info;
    info.abilityName_ = "TestEraseSceneSessionAndMarkDirtyLocked_02";
    info.bundleName_ = "TestEraseSceneSessionAndMarkDirtyLocked_02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    const int32_t validId = 100;
    ssm_->sceneSessionMap_.insert({ validId, sceneSession });

    // erase invisible session
    sceneSession->isVisible_ = false;
    ssm_->EraseSceneSessionAndMarkDirtyLocked(validId);
    ASSERT_EQ(ssm_->sessionMapDirty_, 0);
    ASSERT_EQ(ssm_->sceneSessionMap_.find(validId), ssm_->sceneSessionMap_.end());
}

/**
 * @tc.name: TestEraseSceneSessionAndMarkDirtyLocked_03
 * @tc.desc: Test EraseSceneSessionAndMarkDirtyLocked with erase visible session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestEraseSceneSessionAndMarkDirtyLocked_03, TestSize.Level1)
{
    // init
    ssm_->sceneSessionMap_.clear();
    ssm_->sessionMapDirty_ = 0;

    SessionInfo info;
    info.abilityName_ = "TestEraseSceneSessionAndMarkDirtyLocked_03";
    info.bundleName_ = "TestEraseSceneSessionAndMarkDirtyLocked_03";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    const int32_t validId = 100;

    // erase visible session
    ssm_->sceneSessionMap_.insert({ validId, sceneSession });
    sceneSession->isVisible_ = true;
    ssm_->EraseSceneSessionAndMarkDirtyLocked(validId);
    ASSERT_EQ(ssm_->sessionMapDirty_, static_cast<uint32_t>(SessionUIDirtyFlag::VISIBLE));
    ASSERT_EQ(ssm_->sceneSessionMap_.find(validId), ssm_->sceneSessionMap_.end());
}

/**
 * @tc.name: ProcessUpdateLastFocusedAppId
 * @tc.desc: test ProcessUpdateLastFocusedAppId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, ProcessUpdateLastFocusedAppId, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    std::vector<std::pair<uint32_t, uint32_t>> zOrderList;
    ssm_->windowFocusController_->UpdateFocusedAppSessionId(DEFAULT_DISPLAY_ID, INVALID_SESSION_ID);
    ssm_->ProcessUpdateLastFocusedAppId(zOrderList);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "lastFocusedAppSession";
    sessionInfo.abilityName_ = "lastFocusedAppSession";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    ssm_->windowFocusController_->UpdateFocusedAppSessionId(DEFAULT_DISPLAY_ID, 1);
    sceneSession->zOrder_ = 101;
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);

    ssm_->ProcessUpdateLastFocusedAppId(zOrderList);
    ASSERT_EQ(1, focusGroup->GetLastFocusedAppSessionId());

    zOrderList.push_back(std::make_pair(100, 103));
    ssm_->ProcessUpdateLastFocusedAppId(zOrderList);
    ASSERT_EQ(INVALID_SESSION_ID, focusGroup->GetLastFocusedAppSessionId());
}

/**
 * @tc.name: TestIsNeedSkipWindowModeTypeCheck_01
 * @tc.desc: Test IsNeedSkipWindowModeTypeCheck with sceneSession nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestIsNeedSkipWindowModeTypeCheck_01, TestSize.Level1)
{
    sptr<SceneSession> sceneSession;
    auto ret = ssm_->IsNeedSkipWindowModeTypeCheck(sceneSession, false);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: TestIsNeedSkipWindowModeTypeCheck_02
 * @tc.desc: Test IsNeedSkipWindowModeTypeCheck with sceneSession not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestIsNeedSkipWindowModeTypeCheck_02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "IsNeedSkipWindowModeTypeCheck";
    sessionInfo.abilityName_ = "IsNeedSkipWindowModeTypeCheck";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sceneSession->SetRSVisible(true);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    auto ret = ssm_->IsNeedSkipWindowModeTypeCheck(sceneSession, false);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: TestIsNeedSkipWindowModeTypeCheck_03
 * @tc.desc: Test IsNeedSkipWindowModeTypeCheck WindowType APP_MAIN_WINDOW_BASE
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestIsNeedSkipWindowModeTypeCheck_03, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "IsNeedSkipWindowModeTypeCheck";
    sessionInfo.abilityName_ = "IsNeedSkipWindowModeTypeCheck";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->SetRSVisible(false);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    auto ret = ssm_->IsNeedSkipWindowModeTypeCheck(sceneSession, false);
    ASSERT_TRUE(ret);
}

/**
 * @tc.name: TestIsNeedSkipWindowModeTypeCheck_04
 * @tc.desc: Test IsNeedSkipWindowModeTypeCheck WindowType APP_MAIN_WINDOW_BASE and RSVisible true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, TestIsNeedSkipWindowModeTypeCheck_04, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "TestIsNeedSkipWindowModeTypeCheck_04";
    sessionInfo.abilityName_ = "TestIsNeedSkipWindowModeTypeCheck_04";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->SetRSVisible(true);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    DisplayId displayId = 1001;
    sceneSession->property_->SetDisplayId(displayId);
    auto ret = ssm_->IsNeedSkipWindowModeTypeCheck(sceneSession, true);
    EXPECT_TRUE(ret);

    ret = ssm_->IsNeedSkipWindowModeTypeCheck(sceneSession, false);
    ASSERT_FALSE(ret);
}

/**
 * @tc.name: NotifyStatusBarShowStatus
 * @tc.desc: test NotifyStatusBarShowStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, NotifyStatusBarShowStatus, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);

    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    sceneSession->isStatusBarVisible_ = true;
    EXPECT_EQ(WSError::WS_OK, ssm_->NotifyStatusBarShowStatus(sceneSession->GetPersistentId(), false));
    ssm_->sceneSessionMap_.erase(sceneSession->GetPersistentId());
}

/**
 * @tc.name: GetStatusBarConstantlyShow
 * @tc.desc: test GetStatusBarConstantlyShow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, GetStatusBarConstantlyShow, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<RootSceneSession> rootSceneSession = sptr<RootSceneSession>::MakeSptr();
    ASSERT_NE(nullptr, rootSceneSession);
    sceneSession->property_->SetPersistentId(1);
    rootSceneSession->property_->SetPersistentId(2);
    ssm_->rootSceneSession_ = rootSceneSession;

    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    bool isVisible;
    ssm_->NotifyStatusBarConstantlyShow(sceneSession->GetSessionProperty()->GetDisplayId(), false);
    ssm_->GetStatusBarConstantlyShow(sceneSession->GetSessionProperty()->GetDisplayId(), isVisible);
    EXPECT_EQ(isVisible, false);
    ssm_->sceneSessionMap_.erase(sceneSession->GetPersistentId());
}

/**
 * @tc.name: NotifyAppUseControlList
 * @tc.desc: NotifyAppUseControlList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, NotifyAppUseControlList, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    std::vector<AppUseControlInfo> controlList;
    controlList.emplace_back();
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PERMISSION,
              ssm_->NotifyAppUseControlList(ControlAppType::APP_LOCK, -1, controlList));

    AppUseControlInfo appUseControlInfo;
    appUseControlInfo.bundleName_ = "bundleName";
    appUseControlInfo.appIndex_ = 1;
    appUseControlInfo.isNeedControl_ = true;
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PERMISSION,
              ssm_->NotifyAppUseControlList(ControlAppType::APP_LOCK, -1, controlList));
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: NotifyAppUseControlListInner
 * @tc.desc: NotifyAppUseControlListInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, NotifyAppUseControlListInner, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    int32_t controlListSize = -1;
    ssm_->notifyAppUseControlListFunc_ = [&controlListSize]
        (ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList) {
            controlListSize = controlList.size();
        };
    AppUseControlInfo controlById;
    controlById.persistentId_ = 100;
    controlById.bundleName_ = "bundleName";
    controlById.appIndex_ = 0;
    std::vector<AppUseControlInfo> controlList;
    controlList.push_back(controlById);
    ssm_->NotifyAppUseControlListInner(ControlAppType::DLP, 0, controlList);
    EXPECT_EQ(controlListSize, -1);
    ssm_->sceneSessionMap_.emplace(100, nullptr);
    ssm_->NotifyAppUseControlListInner(ControlAppType::DLP, 0, controlList);
    EXPECT_EQ(controlListSize, -1);

    SessionInfo sessionInfo;
    sessionInfo.persistentId_ = 100;
    sessionInfo.bundleName_ = "bundleName";
    sessionInfo.appIndex_ = 0;
    sessionInfo.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ssm_->sceneSessionMap_.emplace(100, sceneSession);

    ssm_->NotifyAppUseControlListInner(ControlAppType::DLP, 0, controlList);
    EXPECT_EQ(controlListSize, -1);

    AppUseControlInfo controlByBundle;
    controlByBundle.bundleName_ = "bundleName";
    controlByBundle.appIndex_ = 0;
    controlList.push_back(controlByBundle);
    ssm_->NotifyAppUseControlListInner(ControlAppType::DLP, 0, controlList);
    EXPECT_EQ(controlListSize, 1);
}

/**
 * @tc.name: NotifyAppUseControlListInner02
 * @tc.desc: NotifyAppUseControlListInner02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, NotifyAppUseControlListInner02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    int32_t controlListSize = -1;
    ssm_->notifyAppUseControlListFunc_ = [&controlListSize]
        (ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList) {
            controlListSize = controlList.size();
        };
    std::vector<AppUseControlInfo> controlList;
    AppUseControlInfo controlById;
    controlById.persistentId_ = 100;
    controlById.bundleName_ = "bundleName";
    controlById.appIndex_ = 0;
    controlList.push_back(controlById);
    AppUseControlInfo controlById2;
    controlById2.persistentId_ = 100;
    controlById2.bundleName_ = "errorBundleName";
    controlById2.appIndex_ = 0;
    controlList.push_back(controlById2);
    AppUseControlInfo controlById3;
    controlById3.persistentId_ = 100;
    controlById3.bundleName_ = "bundleName";
    controlById3.appIndex_ = 100;
    controlList.push_back(controlById3);

    SessionInfo sessionInfo;
    sessionInfo.persistentId_ = 100;
    sessionInfo.bundleName_ = "bundleName";
    sessionInfo.appIndex_ = 0;
    sessionInfo.windowType_ = 1000;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ssm_->sceneSessionMap_.emplace(100, sceneSession);
    ssm_->NotifyAppUseControlListInner(ControlAppType::DLP, 0, controlList);
    EXPECT_EQ(controlListSize, -1);
}

/**
 * @tc.name: NotifyAppUseControlListInner03
 * @tc.desc: NotifyAppUseControlListInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, NotifyAppUseControlListInner03, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    int32_t controlListSize = -1;
    ssm_->notifyAppUseControlListFunc_ = [&controlListSize]
        (ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList) {
            controlListSize = controlList.size();
        };
    AppUseControlInfo controlById;
    controlById.persistentId_ = 100;
    controlById.bundleName_ = "bundleName";
    controlById.appIndex_ = 0;
    std::vector<AppUseControlInfo> controlList;
    controlList.push_back(controlById);
    ssm_->NotifyAppUseControlListInner(ControlAppType::APP_LOCK, 0, controlList);
    EXPECT_EQ(controlListSize, -1);
    ssm_->sceneSessionMap_.emplace(100, nullptr);
    ssm_->NotifyAppUseControlListInner(ControlAppType::APP_LOCK, 0, controlList);
    EXPECT_EQ(controlListSize, -1);

    SessionInfo sessionInfo;
    sessionInfo.persistentId_ = 100;
    sessionInfo.bundleName_ = "bundleName";
    sessionInfo.appIndex_ = 0;
    sessionInfo.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ssm_->sceneSessionMap_.emplace(100, sceneSession);

    ssm_->NotifyAppUseControlListInner(ControlAppType::APP_LOCK, 0, controlList);
    EXPECT_EQ(controlListSize, -1);

    AppUseControlInfo controlByBundle;
    controlByBundle.bundleName_ = "bundleName";
    controlByBundle.appIndex_ = 0;
    controlList.push_back(controlByBundle);
    ssm_->NotifyAppUseControlListInner(ControlAppType::APP_LOCK, 0, controlList);
    EXPECT_EQ(controlListSize, 1);
}

/**
 * @tc.name: NotifyAppUseControlListInner04
 * @tc.desc: NotifyAppUseControlListInner04
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, NotifyAppUseControlListInner04, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    int32_t controlListSize = -1;
    ssm_->notifyAppUseControlListFunc_ = [&controlListSize]
        (ControlAppType type, int32_t userId, const std::vector<AppUseControlInfo>& controlList) {
            controlListSize = controlList.size();
        };
    std::vector<AppUseControlInfo> controlList;
    AppUseControlInfo controlById;
    controlById.persistentId_ = 100;
    controlById.bundleName_ = "bundleName";
    controlById.appIndex_ = 0;
    controlList.push_back(controlById);
    AppUseControlInfo controlById2;
    controlById2.persistentId_ = 100;
    controlById2.bundleName_ = "errorBundleName";
    controlById2.appIndex_ = 0;
    controlList.push_back(controlById2);
    AppUseControlInfo controlById3;
    controlById3.persistentId_ = 100;
    controlById3.bundleName_ = "bundleName";
    controlById3.appIndex_ = 100;
    controlList.push_back(controlById3);

    SessionInfo sessionInfo;
    sessionInfo.persistentId_ = 100;
    sessionInfo.bundleName_ = "bundleName";
    sessionInfo.appIndex_ = 0;
    sessionInfo.windowType_ = 1000;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ssm_->sceneSessionMap_.emplace(100, sceneSession);
    ssm_->NotifyAppUseControlListInner(ControlAppType::APP_LOCK, 0, controlList);
    EXPECT_EQ(controlListSize, -1);
}

/**
 * @tc.name: GetSessionForAppUseControl01
 * @tc.desc: GetSessionForAppUseControl
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, GetSessionForAppUseControl01, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.persistentId_ = 100;
    sessionInfo.bundleName_ = "bundleName";
    sessionInfo.appIndex_ = 0;
    sessionInfo.windowType_ = 1000;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ssm_->sceneSessionMap_.emplace(100, sceneSession);

    AppUseControlInfo controlById;
    controlById.persistentId_ = 100;
    controlById.bundleName_ = "bundleName";
    controlById.appIndex_ = 0;

    auto result = ssm_->GetSessionForAppUseControl(controlById);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetSessionForAppUseControl02
 * @tc.desc: GetSessionForAppUseControl
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, GetSessionForAppUseControl02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.persistentId_ = 100;
    sessionInfo.bundleName_ = "bundleName";
    sessionInfo.appIndex_ = 0;
    sessionInfo.windowType_ = 1000;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ssm_->sceneSessionMap_.emplace(100, sceneSession);

    AppUseControlInfo controlById2;
    controlById2.persistentId_ = 100;
    controlById2.bundleName_ = "errorBundleName";
    controlById2.appIndex_ = 0;

    auto result = ssm_->GetSessionForAppUseControl(controlById2);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: GetSessionForAppUseControl03
 * @tc.desc: GetSessionForAppUseControl
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, GetSessionForAppUseControl03, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.persistentId_ = 100;
    sessionInfo.bundleName_ = "bundleName";
    sessionInfo.appIndex_ = 0;
    sessionInfo.windowType_ = 1000;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ssm_->sceneSessionMap_.emplace(100, sceneSession);

    AppUseControlInfo controlById3;
    controlById3.persistentId_ = 100;
    controlById3.bundleName_ = "bundleName";
    controlById3.appIndex_ = 100;

    auto result = ssm_->GetSessionForAppUseControl(controlById3);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: MinimizeMainSession
 * @tc.desc: test MinimizeMainSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, MinimizeMainSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "MinimizeMainSessionBundle";
    sessionInfo.abilityName_ = "MinimizeMainSessionAbility";
    sessionInfo.appIndex_ = 0;
    sessionInfo.windowType_ = 1;
    sessionInfo.sessionState_ = SessionState::STATE_ACTIVE;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);

    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    int userId = ssm_->currentUserId_.load();
    auto result = ssm_->MinimizeMainSession(sessionInfo.bundleName_, sessionInfo.appIndex_, userId);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, result);

    result = ssm_->MinimizeMainSession(sessionInfo.bundleName_, sessionInfo.appIndex_, 1);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, result);
}

/**
 * @tc.name: ListWindowInfo
 * @tc.desc: WM_ERROR_INVALID_PERMISSION
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, ListWindowInfo01, TestSize.Level1)
{
    WindowInfoOption windowInfoOption;
    std::vector<sptr<WindowInfo>> infos;
    ASSERT_EQ(ssm_->ListWindowInfo(windowInfoOption, infos), WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: FilterForListWindowInfo01
 * @tc.desc: ALL
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, FilterForListWindowInfo01, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    InitTestSceneSessionForListWindowInfo();
    WindowInfoOption windowInfoOption;
    windowInfoOption.windowInfoFilterOption = WindowInfoFilterOption::ALL;
    windowInfoOption.windowInfoTypeOption = WindowInfoTypeOption::ALL;
    windowInfoOption.displayId = DISPLAY_ID_INVALID;
    windowInfoOption.windowId = 0;
    int32_t filterNum = 0;
    for (const auto& [_, sceneSession] : ssm_->sceneSessionMap_) {
        if (ssm_->FilterForListWindowInfo(windowInfoOption, sceneSession)) {
            filterNum++;
        }
    }
    ASSERT_EQ(filterNum, 6);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: FilterForListWindowInfo02
 * @tc.desc: EXCLUDE_SYSTEM
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, FilterForListWindowInfo02, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    InitTestSceneSessionForListWindowInfo();
    WindowInfoOption windowInfoOption;
    windowInfoOption.windowInfoFilterOption = WindowInfoFilterOption::EXCLUDE_SYSTEM;
    windowInfoOption.windowInfoTypeOption = WindowInfoTypeOption::ALL;
    windowInfoOption.displayId = DISPLAY_ID_INVALID;
    windowInfoOption.windowId = 0;
    int32_t filterNum = 0;
    for (const auto& [_, sceneSession] : ssm_->sceneSessionMap_) {
        if (ssm_->FilterForListWindowInfo(windowInfoOption, sceneSession)) {
            filterNum++;
        }
    }
    ASSERT_EQ(filterNum, 5);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: FilterForListWindowInfo03
 * @tc.desc: VISIBLE
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, FilterForListWindowInfo03, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    InitTestSceneSessionForListWindowInfo();
    WindowInfoOption windowInfoOption;
    windowInfoOption.windowInfoFilterOption = WindowInfoFilterOption::VISIBLE;
    windowInfoOption.windowInfoTypeOption = WindowInfoTypeOption::ALL;
    windowInfoOption.displayId = DISPLAY_ID_INVALID;
    windowInfoOption.windowId = 0;
    int32_t filterNum = 0;
    for (const auto& [_, sceneSession] : ssm_->sceneSessionMap_) {
        if (ssm_->FilterForListWindowInfo(windowInfoOption, sceneSession)) {
            filterNum++;
        }
    }
    ASSERT_EQ(filterNum, 4);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: FilterForListWindowInfo04
 * @tc.desc: FOREGROUND
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, FilterForListWindowInfo04, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    InitTestSceneSessionForListWindowInfo();
    WindowInfoOption windowInfoOption;
    windowInfoOption.windowInfoFilterOption = WindowInfoFilterOption::FOREGROUND;
    windowInfoOption.windowInfoTypeOption = WindowInfoTypeOption::ALL;
    windowInfoOption.displayId = DISPLAY_ID_INVALID;
    windowInfoOption.windowId = 0;
    int32_t filterNum = 0;
    for (const auto& [_, sceneSession] : ssm_->sceneSessionMap_) {
        if (ssm_->FilterForListWindowInfo(windowInfoOption, sceneSession)) {
            filterNum++;
        }
    }
    ASSERT_EQ(filterNum, 5);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: FilterForListWindowInfo05
 * @tc.desc: EXCLUDE_SYSTEM | VISIBLE
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, FilterForListWindowInfo05, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    InitTestSceneSessionForListWindowInfo();
    WindowInfoOption windowInfoOption;
    windowInfoOption.windowInfoFilterOption = WindowInfoFilterOption::EXCLUDE_SYSTEM | WindowInfoFilterOption::VISIBLE;
    windowInfoOption.windowInfoTypeOption = WindowInfoTypeOption::ALL;
    windowInfoOption.displayId = DISPLAY_ID_INVALID;
    windowInfoOption.windowId = 0;
    int32_t filterNum = 0;
    for (const auto& [_, sceneSession] : ssm_->sceneSessionMap_) {
        if (ssm_->FilterForListWindowInfo(windowInfoOption, sceneSession)) {
            filterNum++;
        }
    }
    ASSERT_EQ(filterNum, 3);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: FilterForListWindowInfo06
 * @tc.desc: EXCLUDE_SYSTEM | FOREGROUND
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, FilterForListWindowInfo06, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    InitTestSceneSessionForListWindowInfo();
    WindowInfoOption windowInfoOption;
    windowInfoOption.windowInfoFilterOption =
        WindowInfoFilterOption::EXCLUDE_SYSTEM | WindowInfoFilterOption::FOREGROUND;
    windowInfoOption.windowInfoTypeOption = WindowInfoTypeOption::ALL;
    windowInfoOption.displayId = DISPLAY_ID_INVALID;
    windowInfoOption.windowId = 0;
    int32_t filterNum = 0;
    for (const auto& [_, sceneSession] : ssm_->sceneSessionMap_) {
        if (ssm_->FilterForListWindowInfo(windowInfoOption, sceneSession)) {
            filterNum++;
        }
    }
    ASSERT_EQ(filterNum, 4);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: FilterForListWindowInfo07
 * @tc.desc: FOREGROUND | VISIBLE
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, FilterForListWindowInfo07, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    InitTestSceneSessionForListWindowInfo();
    WindowInfoOption windowInfoOption;
    windowInfoOption.windowInfoFilterOption = WindowInfoFilterOption::FOREGROUND | WindowInfoFilterOption::VISIBLE;
    windowInfoOption.windowInfoTypeOption = WindowInfoTypeOption::ALL;
    windowInfoOption.displayId = DISPLAY_ID_INVALID;
    windowInfoOption.windowId = 0;
    int32_t filterNum = 0;
    for (const auto& [_, sceneSession] : ssm_->sceneSessionMap_) {
        if (ssm_->FilterForListWindowInfo(windowInfoOption, sceneSession)) {
            filterNum++;
        }
    }
    ASSERT_EQ(filterNum, 4);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: FilterForListWindowInfo08
 * @tc.desc: displayId = 0
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, FilterForListWindowInfo08, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    InitTestSceneSessionForListWindowInfo();
    WindowInfoOption windowInfoOption;
    windowInfoOption.windowInfoFilterOption = WindowInfoFilterOption::ALL;
    windowInfoOption.windowInfoTypeOption = WindowInfoTypeOption::ALL;
    windowInfoOption.displayId = 0;
    windowInfoOption.windowId = 0;
    int32_t filterNum = 0;
    for (const auto& [_, sceneSession] : ssm_->sceneSessionMap_) {
        if (ssm_->FilterForListWindowInfo(windowInfoOption, sceneSession)) {
            filterNum++;
        }
    }
    ASSERT_EQ(filterNum, 5);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: NotifyNextAvoidRectInfo_statusBar
 * @tc.desc: SceneSesionManager test NotifyNextAvoidRectInfo_statusBar
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, NotifyNextAvoidRectInfo_statusBar, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    WSRect portraitRect = { 0, 0, 1260, 123 };
    WSRect landspaceRect = { 0, 0, 2720, 123 };
    auto ret = ssm_->NotifyNextAvoidRectInfo(AvoidAreaType::TYPE_SYSTEM, portraitRect, landspaceRect, 0);
    ASSERT_EQ(ret, WSError::WS_OK);
    SessionInfo info;
    info.abilityName_ = "NotifyNextAvoidRectInfo_statusBar";
    info.bundleName_ = "NotifyNextAvoidRectInfo_statusBar";
    info.screenId_ = 0;
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onGetNextAvoidAreaRectInfo_ =
        [](DisplayId displayId, AvoidAreaType type, std::pair<WSRect, WSRect>& nextSystemBarAvoidAreaRectInfo) {
            return ssm_->GetNextAvoidRectInfo(displayId, type, nextSystemBarAvoidAreaRectInfo);
        };
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sceneSession->property_->SetPersistentId(1);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sceneSession->GetLayoutController()->SetSessionRect({ 0, 0, 1260, 2720 });
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    std::map<WindowType, SystemBarProperty> properties;
    properties[WindowType::WINDOW_TYPE_STATUS_BAR] = SystemBarProperty();
    properties[WindowType::WINDOW_TYPE_STATUS_BAR].settingFlag_ = SystemBarSettingFlag::ENABLE_SETTING;
    properties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR] = SystemBarProperty();
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_0, { 0, 0, 1260, 2720 }, properties, avoidAreas);
    Rect rect = { 0, 0, 1260, 123 };
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_SYSTEM].topRect_, rect);
    properties[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = false;
    rect = { 0, 0, 0, 0 };
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_0, { 0, 0, 1260, 123 }, properties, avoidAreas);
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_SYSTEM].topRect_, rect);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: NotifyNextAvoidRectInfo_statusBar_01
 * @tc.desc: SceneSesionManager test NotifyNextAvoidRectInfo_statusBar_01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, NotifyNextAvoidRectInfo_statusBar_01, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    WSRect portraitRect = { 0, 0, 1260, 123 };
    WSRect landspaceRect = { 0, 0, 2720, 123 };
    auto ret = ssm_->NotifyNextAvoidRectInfo(AvoidAreaType::TYPE_SYSTEM, portraitRect, landspaceRect, 0);
    ASSERT_EQ(ret, WSError::WS_OK);
    SessionInfo info;
    info.abilityName_ = "NotifyNextAvoidRectInfo_statusBar_01";
    info.bundleName_ = "NotifyNextAvoidRectInfo_statusBar_01";
    info.screenId_ = 0;
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onGetNextAvoidAreaRectInfo_ =
        [](DisplayId displayId, AvoidAreaType type, std::pair<WSRect, WSRect>& nextSystemBarAvoidAreaRectInfo) {
            return ssm_->GetNextAvoidRectInfo(displayId, type, nextSystemBarAvoidAreaRectInfo);
        };
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sceneSession->property_->SetPersistentId(1);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sceneSession->GetLayoutController()->SetSessionRect({ 0, 0, 1260, 2720 });
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    std::map<WindowType, SystemBarProperty> properties;
    properties[WindowType::WINDOW_TYPE_STATUS_BAR] = SystemBarProperty();
    properties[WindowType::WINDOW_TYPE_STATUS_BAR].settingFlag_ = SystemBarSettingFlag::ENABLE_SETTING;
    properties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR] = SystemBarProperty();
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_90, { 0, 0, 2720, 1260 }, properties, avoidAreas);
    Rect rect = { 0, 0, 2720, 123 };
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_SYSTEM].topRect_, rect);
    properties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].enable_ = false;
    rect = { 0, 0, 0, 0 };
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_90, { 0, 0, 2720, 1260 }, properties, avoidAreas);
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_NAVIGATION_INDICATOR].bottomRect_, rect);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: NotifyNextAvoidRectInfo_keyboard
 * @tc.desc: SceneSesionManager test NotifyNextAvoidRectInfo_keyboard
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, NotifyNextAvoidRectInfo_keyboard, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "NotifyNextAvoidRectInfo_keyboard";
    info.bundleName_ = "NotifyNextAvoidRectInfo_keyboard";
    info.screenId_ = 0;
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onKeyboardRotationChange_ =
        [](int32_t persistentId, Rotation rotation, std::vector<std::pair<bool, WSRect>>& avoidAreas) {
            ssm_->GetKeyboardOccupiedAreaWithRotation(persistentId, rotation, avoidAreas);
        };
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sceneSession->property_->SetPersistentId(1);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    AvoidArea avoidArea;
    sceneSession->GetKeyboardAvoidAreaByRotation(Rotation::ROTATION_0, { 0, 0, 1260, 2720 }, avoidArea);
    Rect rect = { 0, 0, 0, 0 };
    ASSERT_EQ(avoidArea.bottomRect_, rect);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: NotifyNextAvoidRectInfo_keyboard_01
 * @tc.desc: SceneSesionManager test NotifyNextAvoidRectInfo_keyboard_01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, NotifyNextAvoidRectInfo_keyboard_01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "NotifyNextAvoidRectInfo_keyboard_01";
    info.bundleName_ = "NotifyNextAvoidRectInfo_keyboard_01";
    info.screenId_ = 0;
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onKeyboardRotationChange_ =
        [](int32_t persistentId, Rotation rotation, std::vector<std::pair<bool, WSRect>>& avoidAreas) {
            ssm_->GetKeyboardOccupiedAreaWithRotation(persistentId, rotation, avoidAreas);
        };
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sceneSession->property_->SetPersistentId(1);
    SessionInfo keyboardSessionInfo;
    keyboardSessionInfo.abilityName_ = "keyboard";
    keyboardSessionInfo.bundleName_ = "keyboard";
    keyboardSessionInfo.screenId_ = 0;
    sptr<SceneSession> keyboardSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    keyboardSession->property_->type_ = WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT;
    keyboardSession->property_->keyboardLayoutParams_.PortraitPanelRect_ = { 0, 1700, 1260, 1020 };
    keyboardSession->property_->keyboardLayoutParams_.LandscapePanelRect_ = { 0, 538, 2720, 722 };
    keyboardSession->property_->SetPersistentId(2);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ssm_->sceneSessionMap_.insert({ keyboardSession->GetPersistentId(), keyboardSession });
    auto uiType = ssm_->systemConfig_.windowUIType_;
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    AvoidArea avoidArea;
    sceneSession->GetKeyboardAvoidAreaByRotation(Rotation::ROTATION_0, { 0, 0, 1260, 2720 }, avoidArea);
    Rect rect = { 0, 1700, 1260, 1020 };
    ASSERT_EQ(avoidArea.bottomRect_, rect);
    sceneSession->GetKeyboardAvoidAreaByRotation(Rotation::ROTATION_90, { 0, 0, 2720, 1260 }, avoidArea);
    rect = { 0, 538, 2720, 722 };
    ASSERT_EQ(avoidArea.bottomRect_, rect);
    keyboardSession->state_ = SessionState::STATE_BACKGROUND;
    rect = { 0, 0, 0, 0 };
    avoidArea.bottomRect_ = rect;
    sceneSession->GetKeyboardAvoidAreaByRotation(Rotation::ROTATION_180, { 0, 0, 1260, 2720 }, avoidArea);
    ASSERT_EQ(avoidArea.bottomRect_, rect);
    ssm_->systemConfig_.windowUIType_ = uiType;
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: NotifyNextAvoidRectInfo_AIBar
 * @tc.desc: SceneSesionManager test NotifyNextAvoidRectInfo_AIBar
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, NotifyNextAvoidRectInfo_AIBar, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    WSRect portraitRect = { 409, 2629, 442, 91 };
    WSRect landspaceRect = { 884, 1169, 952, 91 };
    auto ret = ssm_->NotifyNextAvoidRectInfo(AvoidAreaType::TYPE_NAVIGATION_INDICATOR, portraitRect, landspaceRect, 0);
    ASSERT_EQ(ret, WSError::WS_OK);
    SessionInfo info;
    info.abilityName_ = "NotifyNextAvoidRectInfo_AIBar";
    info.bundleName_ = "NotifyNextAvoidRectInfo_AIBar";
    info.screenId_ = 0;
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    specificCb->onGetNextAvoidAreaRectInfo_ =
        [](DisplayId displayId, AvoidAreaType type, std::pair<WSRect, WSRect>& nextSystemBarAvoidAreaRectInfo) {
            return ssm_->GetNextAvoidRectInfo(displayId, type, nextSystemBarAvoidAreaRectInfo);
        };
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sceneSession->property_->SetPersistentId(1);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sceneSession->GetLayoutController()->SetSessionRect({ 0, 0, 1260, 2720 });
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    std::map<WindowType, SystemBarProperty> properties;
    properties[WindowType::WINDOW_TYPE_STATUS_BAR] = SystemBarProperty();
    properties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR] = SystemBarProperty();
    properties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].settingFlag_ = SystemBarSettingFlag::ENABLE_SETTING;
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_0, { 0, 0, 1260, 2720 }, properties, avoidAreas);
    Rect rect = { 409, 2629, 442, 91 };
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_NAVIGATION_INDICATOR].bottomRect_, rect);
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_90, { 0, 0, 2720, 1260 }, properties, avoidAreas);
    rect = { 884, 1169, 952, 91 };
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_NAVIGATION_INDICATOR].bottomRect_, rect);
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_180, { 0, 0, 1260, 2720 }, properties, avoidAreas);
    rect = { 409, 2629, 442, 91 };
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_NAVIGATION_INDICATOR].bottomRect_, rect);
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_270, { 0, 0, 2720, 1260 }, properties, avoidAreas);
    rect = { 884, 1169, 952, 91 };
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_NAVIGATION_INDICATOR].bottomRect_, rect);
    properties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].enable_ = false;
    rect = { 0, 0, 0, 0 };
    sceneSession->GetAvoidAreasByRotation(Rotation::ROTATION_0, { 0, 0, 2720, 1260 }, properties, avoidAreas);
    ASSERT_EQ(avoidAreas[AvoidAreaType::TYPE_NAVIGATION_INDICATOR].topRect_, rect);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: NotifyWindowSystemBarPropertyChange
 * @tc.desc: SceneSesionManager test NotifyWindowSystemBarPropertyChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, NotifyWindowSystemBarPropertyChange, TestSize.Level0)
{
    EXPECT_NE(ssm_, nullptr);
    SystemBarProperty systemBarProperty;
    ssm_->NotifyWindowSystemBarPropertyChange(WindowType::WINDOW_TYPE_STATUS_BAR, systemBarProperty);
    EXPECT_EQ(ssm_->lastSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR], systemBarProperty);
    ssm_->NotifyWindowSystemBarPropertyChange(WindowType::WINDOW_TYPE_STATUS_BAR, systemBarProperty);
    EXPECT_EQ(ssm_->lastSystemBarPropertyMap_[WindowType::WINDOW_TYPE_STATUS_BAR], systemBarProperty);
}

/**
 * @tc.name: RefreshAllAppUseControlMap
 * @tc.desc: RefreshAllAppUseControlMap
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, RefreshAllAppUseControlMap, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    AppUseControlInfo appUseControlInfo;
    appUseControlInfo.bundleName_ = "app_bundle_name";
    ssm_->RefreshAllAppUseControlMap(appUseControlInfo, ControlAppType::APP_LOCK);

    appUseControlInfo.isNeedControl_ = true;
    ssm_->RefreshAllAppUseControlMap(appUseControlInfo, ControlAppType::APP_LOCK);
    EXPECT_EQ(1, ssm_->allAppUseControlMap_.size());

    appUseControlInfo.isNeedControl_ = false;
    ssm_->RefreshAllAppUseControlMap(appUseControlInfo, ControlAppType::APP_LOCK);
    EXPECT_EQ(0, ssm_->allAppUseControlMap_.size());

    appUseControlInfo.isNeedControl_ = false;
    appUseControlInfo.isControlRecentOnly_ = true;
    appUseControlInfo.isControlRecentOnly_ = false;
    ssm_->RefreshAllAppUseControlMap(appUseControlInfo, ControlAppType::APP_LOCK);
    ssm_->RefreshAllAppUseControlMap(appUseControlInfo, ControlAppType::PARENT_CONTROL);
    EXPECT_EQ(0, ssm_->allAppUseControlMap_.size());
}

/**
 * @tc.name: SetMaximizeFullScreen
 * @tc.desc: SetMaximizeFullScreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest10, SetMaximizeFullScreen, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    ASSERT_NE(ssm_, nullptr);
    int32_t persistentId = 100;
    bool isMaximizeFullScreen = true;
    EXPECT_EQ(ssm_->SetMaximizeFullScreen(persistentId, isMaximizeFullScreen), WSError::WS_ERROR_INVALID_WINDOW);
 
    SessionInfo info;
    info.abilityName_ = "testSetMaximizeFullScreen1";
    info.bundleName_ = "testSetMaximizeFullScreen1";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->property_->SetPersistentId(200);
    sceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ssm_->sceneSessionMap_.insert({sceneSession->GetPersistentId(), sceneSession});
    EXPECT_EQ(ssm_->SetMaximizeFullScreen(sceneSession->GetPersistentId(), isMaximizeFullScreen),
        WSError::WS_ERROR_INVALID_WINDOW);
 
    SessionInfo info1;
    info1.abilityName_ = "testSetMaximizeFullScreen2";
    info1.bundleName_ = "testSetMaximizeFullScreen2";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    sceneSession1->property_->SetPersistentId(300);
    sceneSession1->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession1->SetSessionState(SessionState::STATE_FOREGROUND);
    ssm_->sceneSessionMap_.insert({sceneSession1->GetPersistentId(), sceneSession1});
    EXPECT_EQ(ssm_->SetMaximizeFullScreen(sceneSession1->GetPersistentId(), isMaximizeFullScreen),
        WSError::WS_ERROR_NULLPTR);
 
    ssm_->sceneSessionMap_.clear();
}
} // namespace
} // namespace Rosen
} // namespace OHOS