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
#include "interfaces/include/ws_common.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session/host/include/scene_session.h"
#include "session/screen/include/screen_session.h"
#include "session_manager.h"
#include "session_manager/include/scene_session_manager.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class SceneSessionManagerFocusTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static sptr<SceneSessionManager> ssm_;
};
sptr<SceneSessionManager> SceneSessionManagerFocusTest::ssm_ = nullptr;

void SceneSessionManagerFocusTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerFocusTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerFocusTest::SetUp() {}

void SceneSessionManagerFocusTest::TearDown() {}

namespace {
/**
 * @tc.name: RemoveFocusGroup
 * @tc.desc: RemoveFocusGroup_Invalid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, RemoveFocusGroup_Invalid, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, ssm_);
    WSError ret = ssm_->RemoveFocusGroup(0, 0);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: AddFocusGroup
 * @tc.desc: AddFocusGroup
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, AddFocusGroup, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, ssm_);
    WSError ret = ssm_->AddFocusGroup(0, 0);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: OnScreenConnected
 * @tc.desc: OnScreenConnected
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, OnScreenConnected, Function | SmallTest | Level2)
{
    auto screenConnectionChangeListener = sptr<ScreenConnectionChangeListener>::MakeSptr();
    ASSERT_NE(nullptr, screenConnectionChangeListener);
    screenConnectionChangeListener->OnScreenConnected(nullptr);

    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    screenConnectionChangeListener->OnScreenConnected(screenSession);

    screenSession->SetName("CeliaView");
    screenSession->SetScreenType(ScreenType::VIRTUAL);
    screenConnectionChangeListener->OnScreenConnected(screenSession);

    screenSession->SetName("DevEcoViewer");
    screenSession->SetScreenType(ScreenType::VIRTUAL);
    screenConnectionChangeListener->OnScreenConnected(screenSession);
}

/**
 * @tc.name: OnScreenDisconnected
 * @tc.desc: OnScreenDisconnected
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, OnScreenDisconnected, Function | SmallTest | Level2)
{
    auto screenConnectionChangeListener = sptr<ScreenConnectionChangeListener>::MakeSptr();
    ASSERT_NE(nullptr, screenConnectionChangeListener);
    screenConnectionChangeListener->OnScreenDisconnected(nullptr);

    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    screenConnectionChangeListener->OnScreenDisconnected(screenSession);

    screenSession->SetName("CeliaView");
    screenSession->SetScreenType(ScreenType::VIRTUAL);
    screenConnectionChangeListener->OnScreenDisconnected(screenSession);

    screenSession->SetName("DevEcoViewer");
    screenSession->SetScreenType(ScreenType::VIRTUAL);
    screenConnectionChangeListener->OnScreenDisconnected(screenSession);
}

/**
 * @tc.name: IsBlockingFocusWindowType_SessionConfig
 * @tc.desc: Check if the session blockingFocus and device type is qualified
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, IsBlockingFocusWindowType_SessionConfig, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsBlockingFocusWindowType_SessionConfig";
    info.bundleName_ = "IsBlockingFocusWindowType_SessionConfig";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->blockingFocus_ = false;
    EXPECT_EQ(false, ssm_->IsBlockingFocusWindowType(sceneSession));

    sceneSession->blockingFocus_ = true;
    sceneSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(false, ssm_->IsBlockingFocusWindowType(sceneSession));

    sceneSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(false, ssm_->IsBlockingFocusWindowType(sceneSession));

    sceneSession->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    EXPECT_EQ(false, ssm_->IsBlockingFocusWindowType(sceneSession));
}

/**
 * @tc.name: IsBlockingFocusWindowType_WindowType
 * @tc.desc: Check if the session blockingFocus with window type
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, IsBlockingFocusWindowType_WindowType, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsBlockingFocusWindowType_WindowType";
    info.bundleName_ = "IsBlockingFocusWindowType_WindowType";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->blockingFocus_ = true;
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    EXPECT_EQ(false, ssm_->IsBlockingFocusWindowType(sceneSession));

    sceneSession->blockingFocus_ = false;
    EXPECT_EQ(false, ssm_->IsBlockingFocusWindowType(sceneSession));

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    EXPECT_EQ(false, ssm_->IsBlockingFocusWindowType(sceneSession));
}

/**
 * @tc.name: IsBlockingFocusWindowType_HotAreasCheck
 * @tc.desc: Check if one of the HotAreas of the current session is full-screen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, IsBlockingFocusWindowType_HotAreasCheck, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsBlockingFocusWindowType_HotAreasCheck";
    info.bundleName_ = "IsBlockingFocusWindowType_HotAreasCheck";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->blockingFocus_ = true;
    sceneSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->GetSessionProperty()->SetDisplayId(10001);
    EXPECT_EQ(false, ssm_->IsBlockingFocusWindowType(sceneSession));

    sceneSession->GetSessionProperty()->SetDisplayId(DEFAULT_DISPLAY_ID);
    auto display = DisplayManager::GetInstance().GetDisplayById(DEFAULT_DISPLAY_ID);
    ASSERT_NE(nullptr, display);
    auto displayInfo = display->GetDisplayInfo();
    ASSERT_NE(nullptr, displayInfo);
    auto width = displayInfo->GetWidth();
    auto height = displayInfo->GetHeight();
    sceneSession->SetSessionRect({ 0, 0, width, height });
    std::vector<OHOS::Rosen::Rect> touchHotAreasInSceneSession(0);
    EXPECT_EQ(false, ssm_->IsBlockingFocusWindowType(sceneSession));

    OHOS::Rosen::Rect area = { 0, 0, width -1, height };
    touchHotAreasInSceneSession.emplace_back(area);
    sceneSession->GetSessionProperty()->SetTouchHotAreas(touchHotAreasInSceneSession);
    EXPECT_EQ(false, ssm_->IsBlockingFocusWindowType(sceneSession));

    area = { 0, 1, width, height };
    touchHotAreasInSceneSession.emplace_back(area);
    sceneSession->GetSessionProperty()->SetTouchHotAreas(touchHotAreasInSceneSession);
    EXPECT_EQ(false, ssm_->IsBlockingFocusWindowType(sceneSession));

    area = { 0, 0, width, height - 1 };
    touchHotAreasInSceneSession.emplace_back(area);
    sceneSession->GetSessionProperty()->SetTouchHotAreas(touchHotAreasInSceneSession);
    EXPECT_EQ(false, ssm_->IsBlockingFocusWindowType(sceneSession));

    area = { 1, 0, width, height };
    touchHotAreasInSceneSession.emplace_back(area);
    sceneSession->GetSessionProperty()->SetTouchHotAreas(touchHotAreasInSceneSession);
    EXPECT_EQ(false, ssm_->IsBlockingFocusWindowType(sceneSession));

    area = { 0, 0, width, height };
    touchHotAreasInSceneSession.emplace_back(area);
    sceneSession->GetSessionProperty()->SetTouchHotAreas(touchHotAreasInSceneSession);
    EXPECT_EQ(true, ssm_->IsBlockingFocusWindowType(sceneSession));
}

/**
 * @tc.name: PostProcessFocus
 * @tc.desc: test function : PostProcessFocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, PostProcessFocus, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerFocusTest, PostProcessFocus01, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerFocusTest, PostProcessFocus03, TestSize.Level1)
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
 * @tc.name: PostProcessFocus04
 * @tc.desc: test PostProcessFocus with multiple sessions sorted by ZOrder
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, PostProcessFocus04, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(0);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessFocus04";
    sessionInfo.abilityName_ = "PostProcessFocus04";

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession1->persistentId_ = 1;
    sceneSession1->SetZOrder(100);
    sceneSession1->SetFocusedOnShow(true);
    sceneSession1->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession1->isVisible_ = true;
    PostProcessFocusState state1 = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession1->SetPostProcessFocusState(state1);

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession2->persistentId_ = 2;
    sceneSession2->SetZOrder(50);
    sceneSession2->SetFocusedOnShow(true);
    sceneSession2->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession2->isVisible_ = true;
    PostProcessFocusState state2 = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession2->SetPostProcessFocusState(state2);

    ssm_->sceneSessionMap_.emplace(1, sceneSession1);
    ssm_->sceneSessionMap_.emplace(2, sceneSession2);
    ssm_->PostProcessFocus();
    EXPECT_EQ(focusGroup->GetFocusedSessionId(), 1);

    ssm_->sceneSessionMap_.erase(1);
    ssm_->sceneSessionMap_.erase(2);
}

/**
 * @tc.name: PostProcessFocus05
 * @tc.desc: test PostProcessFocus with isFocused but not visible
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, PostProcessFocus05, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(0);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessFocus05";
    sessionInfo.abilityName_ = "PostProcessFocus05";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->SetFocusedOnShow(true);
    sceneSession->SetSessionState(SessionState::STATE_BACKGROUND);
    sceneSession->isVisible_ = false;
    PostProcessFocusState state = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession->SetPostProcessFocusState(state);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    ssm_->PostProcessFocus();
    EXPECT_NE(focusGroup->GetFocusedSessionId(), 1);

    ssm_->sceneSessionMap_.erase(1);
}

/**
 * @tc.name: PostProcessFocus06
 * @tc.desc: test PostProcessFocus with SCB_START_APP and DelayFocusChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, PostProcessFocus06, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(0);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessFocus06";
    sessionInfo.abilityName_ = "PostProcessFocus06";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->SetFocusedOnShow(true);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->isVisible_ = true;
    sceneSession->SetHidingStartingWindow(true);
    PostProcessFocusState state = { true, true, true, FocusChangeReason::SCB_START_APP };
    sceneSession->SetPostProcessFocusState(state);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    ssm_->PostProcessFocus();
    EXPECT_NE(focusGroup->GetFocusedSessionId(), 1);

    ssm_->sceneSessionMap_.erase(1);
}

/**
 * @tc.name: PostProcessFocus07
 * @tc.desc: test PostProcessFocus with SCB_START_APP without DelayFocusChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, PostProcessFocus07, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(0);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessFocus07";
    sessionInfo.abilityName_ = "PostProcessFocus07";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->SetFocusedOnShow(true);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->isVisible_ = true;
    sceneSession->SetHidingStartingWindow(false);
    PostProcessFocusState state = { true, true, true, FocusChangeReason::SCB_START_APP };
    sceneSession->SetPostProcessFocusState(state);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    ssm_->PostProcessFocus();
    EXPECT_EQ(focusGroup->GetFocusedSessionId(), 1);

    ssm_->sceneSessionMap_.erase(1);
}

/**
 * @tc.name: PostProcessFocus08
 * @tc.desc: test PostProcessFocus with RECENT reason
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, PostProcessFocus08, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(0);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessFocus08";
    sessionInfo.abilityName_ = "PostProcessFocus08";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->SetFocusedOnShow(true);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->isVisible_ = true;
    PostProcessFocusState state = { true, true, false, FocusChangeReason::RECENT };
    sceneSession->SetPostProcessFocusState(state);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    ssm_->PostProcessFocus();
    EXPECT_EQ(focusGroup->GetFocusedSessionId(), 1);

    ssm_->sceneSessionMap_.erase(1);
}

/**
 * @tc.name: PostProcessFocus09
 * @tc.desc: test PostProcessFocus with isFocused false (unfocus)
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, PostProcessFocus09, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessFocus09";
    sessionInfo.abilityName_ = "PostProcessFocus09";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->SetFocusedOnShow(true);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->isVisible_ = true;
    PostProcessFocusState state = { true, false, true, FocusChangeReason::DEFAULT };
    sceneSession->SetPostProcessFocusState(state);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    ssm_->PostProcessFocus();
    EXPECT_NE(focusGroup->GetFocusedSessionId(), 1);

    ssm_->sceneSessionMap_.erase(1);
}

/**
 * @tc.name: PostProcessFocus10
 * @tc.desc: test PostProcessFocus with same displayGroupId only one focus changed
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, PostProcessFocus10, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(0);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessFocus10";
    sessionInfo.abilityName_ = "PostProcessFocus10";

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession1->persistentId_ = 1;
    sceneSession1->SetFocusedOnShow(true);
    sceneSession1->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession1->isVisible_ = true;
    sceneSession1->SetZOrder(100);
    PostProcessFocusState state1 = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession1->SetPostProcessFocusState(state1);

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession2->persistentId_ = 2;
    sceneSession2->SetFocusedOnShow(true);
    sceneSession2->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession2->isVisible_ = true;
    sceneSession2->SetZOrder(50);
    PostProcessFocusState state2 = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession2->SetPostProcessFocusState(state2);

    ssm_->sceneSessionMap_.emplace(1, sceneSession1);
    ssm_->sceneSessionMap_.emplace(2, sceneSession2);
    ssm_->PostProcessFocus();
    EXPECT_EQ(focusGroup->GetFocusedSessionId(), 1);
    EXPECT_EQ(sceneSession1->GetPostProcessFocusState().enabled_, false);
    EXPECT_EQ(sceneSession2->GetPostProcessFocusState().enabled_, false);

    ssm_->sceneSessionMap_.erase(1);
    ssm_->sceneSessionMap_.erase(2);
}

/**
 * @tc.name: PostProcessFocus11
 * @tc.desc: test PostProcessFocus with non-default displayGroupId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, PostProcessFocus11, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    DisplayGroupId testGroupId = 100;
    DisplayId testDisplayId = 100;
    ssm_->windowFocusController_->AddFocusGroup(testGroupId, testDisplayId);

    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(testDisplayId);
    focusGroup->SetFocusedSessionId(0);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessFocus11";
    sessionInfo.abilityName_ = "PostProcessFocus11";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->SetFocusedOnShow(true);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->isVisible_ = true;
    sceneSession->property_->SetDisplayId(testDisplayId);
    PostProcessFocusState state = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession->SetPostProcessFocusState(state);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    ssm_->PostProcessFocus();
    EXPECT_EQ(focusGroup->GetFocusedSessionId(), 1);

    ssm_->sceneSessionMap_.erase(1);
    ssm_->windowFocusController_->RemoveFocusGroup(testGroupId, testDisplayId);
}

/**
 * @tc.name: PostProcessFocus12
 * @tc.desc: test PostProcessFocus with multiple displayGroupIds
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, PostProcessFocus12, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    DisplayGroupId testGroupId1 = 100;
    DisplayId testDisplayId1 = 100;
    DisplayGroupId testGroupId2 = 200;
    DisplayId testDisplayId2 = 200;
    ssm_->windowFocusController_->AddFocusGroup(testGroupId1, testDisplayId1);
    ssm_->windowFocusController_->AddFocusGroup(testGroupId2, testDisplayId2);

    auto focusGroup1 = ssm_->windowFocusController_->GetFocusGroup(testDisplayId1);
    focusGroup1->SetFocusedSessionId(0);
    auto focusGroup2 = ssm_->windowFocusController_->GetFocusGroup(testDisplayId2);
    focusGroup2->SetFocusedSessionId(0);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessFocus12";
    sessionInfo.abilityName_ = "PostProcessFocus12";

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession1->persistentId_ = 1;
    sceneSession1->SetFocusedOnShow(true);
    sceneSession1->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession1->isVisible_ = true;
    sceneSession1->property_->SetDisplayId(testDisplayId1);
    PostProcessFocusState state1 = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession1->SetPostProcessFocusState(state1);

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession2->persistentId_ = 2;
    sceneSession2->SetFocusedOnShow(true);
    sceneSession2->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession2->isVisible_ = true;
    sceneSession2->property_->SetDisplayId(testDisplayId2);
    PostProcessFocusState state2 = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession2->SetPostProcessFocusState(state2);

    ssm_->sceneSessionMap_.emplace(1, sceneSession1);
    ssm_->sceneSessionMap_.emplace(2, sceneSession2);
    ssm_->PostProcessFocus();
    EXPECT_EQ(focusGroup1->GetFocusedSessionId(), 1);
    EXPECT_EQ(focusGroup2->GetFocusedSessionId(), 2);

    ssm_->sceneSessionMap_.erase(1);
    ssm_->sceneSessionMap_.erase(2);
    ssm_->windowFocusController_->RemoveFocusGroup(testGroupId1, testDisplayId1);
    ssm_->windowFocusController_->RemoveFocusGroup(testGroupId2, testDisplayId2);
}

/**
 * @tc.name: PostProcessFocus13
 * @tc.desc: test PostProcessFocus with nullptr session in processingSessions
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, PostProcessFocus13, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.emplace(0, nullptr);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessFocus13";
    sessionInfo.abilityName_ = "PostProcessFocus13";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->SetFocusedOnShow(true);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->isVisible_ = true;
    PostProcessFocusState state = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession->SetPostProcessFocusState(state);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);

    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(0);
    ssm_->PostProcessFocus();
    EXPECT_EQ(focusGroup->GetFocusedSessionId(), 1);

    ssm_->sceneSessionMap_.erase(0);
    ssm_->sceneSessionMap_.erase(1);
}

/**
 * @tc.name: PostProcessFocus14
 * @tc.desc: test PostProcessFocus with focusCmp sorting (isFocused priority)
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, PostProcessFocus14, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(0);

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PostProcessFocus14";
    sessionInfo.abilityName_ = "PostProcessFocus14";

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession1->persistentId_ = 1;
    sceneSession1->SetFocusedOnShow(true);
    sceneSession1->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession1->isVisible_ = true;
    sceneSession1->SetZOrder(50);
    PostProcessFocusState state1 = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession1->SetPostProcessFocusState(state1);

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession2->persistentId_ = 2;
    sceneSession2->SetFocusedOnShow(true);
    sceneSession2->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession2->isVisible_ = true;
    sceneSession2->SetZOrder(100);
    PostProcessFocusState state2 = { true, false, true, FocusChangeReason::DEFAULT };
    sceneSession2->SetPostProcessFocusState(state2);

    ssm_->sceneSessionMap_.emplace(1, sceneSession1);
    ssm_->sceneSessionMap_.emplace(2, sceneSession2);
    ssm_->PostProcessFocus();
    EXPECT_EQ(focusGroup->GetFocusedSessionId(), 1);

    ssm_->sceneSessionMap_.erase(1);
    ssm_->sceneSessionMap_.erase(2);
}
} // namespace
} // namespace Rosen
} // namespace OHOS