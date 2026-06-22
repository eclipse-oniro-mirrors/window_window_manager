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

/**
 * @tc.name: CollectProcessingSessions01
 * @tc.desc: test CollectProcessingSessions with empty sceneSessionMap
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CollectProcessingSessions01, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    auto sessions = ssm_->CollectProcessingSessions();
    EXPECT_TRUE(sessions.empty());
}

/**
 * @tc.name: CollectProcessingSessions02
 * @tc.desc: test CollectProcessingSessions with nullptr session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CollectProcessingSessions02, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.emplace(0, nullptr);
    auto sessions = ssm_->CollectProcessingSessions();
    EXPECT_TRUE(sessions.empty());
    ssm_->sceneSessionMap_.erase(0);
}

/**
 * @tc.name: CollectProcessingSessions03
 * @tc.desc: test CollectProcessingSessions with disabled state
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CollectProcessingSessions03, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CollectProcessingSessions03";
    sessionInfo.abilityName_ = "CollectProcessingSessions03";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    PostProcessFocusState state = { false, true, true, FocusChangeReason::FOREGROUND };
    sceneSession->SetPostProcessFocusState(state);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    
    auto sessions = ssm_->CollectProcessingSessions();
    EXPECT_TRUE(sessions.empty());
    ssm_->sceneSessionMap_.erase(1);
}

/**
 * @tc.name: CollectProcessingSessions04
 * @tc.desc: test CollectProcessingSessions with isFocused but not visible
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CollectProcessingSessions04, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CollectProcessingSessions04";
    sessionInfo.abilityName_ = "CollectProcessingSessions04";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->isVisible_ = false;
    PostProcessFocusState state = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession->SetPostProcessFocusState(state);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    
    auto sessions = ssm_->CollectProcessingSessions();
    EXPECT_TRUE(sessions.empty());
    ssm_->sceneSessionMap_.erase(1);
}

/**
 * @tc.name: CollectProcessingSessions05
 * @tc.desc: test CollectProcessingSessions with valid session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CollectProcessingSessions05, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CollectProcessingSessions05";
    sessionInfo.abilityName_ = "CollectProcessingSessions05";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->isVisible_ = true;
    PostProcessFocusState state = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession->SetPostProcessFocusState(state);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    
    auto sessions = ssm_->CollectProcessingSessions();
    EXPECT_EQ(sessions.size(), 1);
    ssm_->sceneSessionMap_.erase(1);
}

/**
 * @tc.name: CollectProcessingSessions06
 * @tc.desc: test CollectProcessingSessions with multiple sessions sorted by ZOrder
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CollectProcessingSessions06, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CollectProcessingSessions06";
    sessionInfo.abilityName_ = "CollectProcessingSessions06";

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession1->persistentId_ = 1;
    sceneSession1->SetZOrder(100);
    sceneSession1->isVisible_ = true;
    PostProcessFocusState state1 = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession1->SetPostProcessFocusState(state1);

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession2->persistentId_ = 2;
    sceneSession2->SetZOrder(50);
    sceneSession2->isVisible_ = true;
    PostProcessFocusState state2 = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession2->SetPostProcessFocusState(state2);

    ssm_->sceneSessionMap_.emplace(1, sceneSession1);
    ssm_->sceneSessionMap_.emplace(2, sceneSession2);

    auto sessions = ssm_->CollectProcessingSessions();
    EXPECT_EQ(sessions.size(), 2);
    EXPECT_EQ(sessions[0]->GetZOrder(), 100);
    EXPECT_EQ(sessions[1]->GetZOrder(), 50);

    ssm_->sceneSessionMap_.erase(1);
    ssm_->sceneSessionMap_.erase(2);
}

/**
 * @tc.name: CollectProcessingSessions07
 * @tc.desc: test CollectProcessingSessions with isFocused priority in sorting
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CollectProcessingSessions07, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CollectProcessingSessions07";
    sessionInfo.abilityName_ = "CollectProcessingSessions07";

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession1->persistentId_ = 1;
    sceneSession1->SetZOrder(50);
    sceneSession1->isVisible_ = true;
    PostProcessFocusState state1 = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession1->SetPostProcessFocusState(state1);

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession2->persistentId_ = 2;
    sceneSession2->SetZOrder(100);
    sceneSession2->isVisible_ = true;
    PostProcessFocusState state2 = { true, false, true, FocusChangeReason::DEFAULT };
    sceneSession2->SetPostProcessFocusState(state2);

    ssm_->sceneSessionMap_.emplace(1, sceneSession1);
    ssm_->sceneSessionMap_.emplace(2, sceneSession2);

    auto sessions = ssm_->CollectProcessingSessions();
    EXPECT_EQ(sessions.size(), 2);
    EXPECT_EQ(sessions[0]->GetPersistentId(), 1);

    ssm_->sceneSessionMap_.erase(1);
    ssm_->sceneSessionMap_.erase(2);
}

/**
 * @tc.name: CollectProcessingSessions08
 * @tc.desc: test CollectProcessingSessions with mix of nullptr and valid sessions
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CollectProcessingSessions08, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CollectProcessingSessions08";
    sessionInfo.abilityName_ = "CollectProcessingSessions08";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->isVisible_ = true;
    PostProcessFocusState state = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession->SetPostProcessFocusState(state);

    ssm_->sceneSessionMap_.emplace(0, nullptr);
    ssm_->sceneSessionMap_.emplace(1, sceneSession);
    ssm_->sceneSessionMap_.emplace(2, nullptr);

    auto sessions = ssm_->CollectProcessingSessions();
    EXPECT_EQ(sessions.size(), 1);
    EXPECT_EQ(sessions[0]->GetPersistentId(), 1);

    ssm_->sceneSessionMap_.erase(0);
    ssm_->sceneSessionMap_.erase(1);
    ssm_->sceneSessionMap_.erase(2);
}

/**
 * @tc.name: CollectProcessingSessions09
 * @tc.desc: test CollectProcessingSessions returns sorted vector
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CollectProcessingSessions09, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CollectProcessingSessions09";
    sessionInfo.abilityName_ = "CollectProcessingSessions09";

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession1->persistentId_ = 1;
    sceneSession1->SetZOrder(30);
    sceneSession1->isVisible_ = true;
    PostProcessFocusState state1 = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession1->SetPostProcessFocusState(state1);

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession2->persistentId_ = 2;
    sceneSession2->SetZOrder(60);
    sceneSession2->isVisible_ = true;
    PostProcessFocusState state2 = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession2->SetPostProcessFocusState(state2);

    sptr<SceneSession> sceneSession3 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession3->persistentId_ = 3;
    sceneSession3->SetZOrder(90);
    sceneSession3->isVisible_ = true;
    PostProcessFocusState state3 = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession3->SetPostProcessFocusState(state3);

    ssm_->sceneSessionMap_.emplace(1, sceneSession1);
    ssm_->sceneSessionMap_.emplace(2, sceneSession2);
    ssm_->sceneSessionMap_.emplace(3, sceneSession3);

    auto sessions = ssm_->CollectProcessingSessions();
    EXPECT_EQ(sessions.size(), 3);
    EXPECT_EQ(sessions[0]->GetZOrder(), 90);
    EXPECT_EQ(sessions[1]->GetZOrder(), 60);
    EXPECT_EQ(sessions[2]->GetZOrder(), 30);

    ssm_->sceneSessionMap_.erase(1);
    ssm_->sceneSessionMap_.erase(2);
    ssm_->sceneSessionMap_.erase(3);
}

/**
 * @tc.name: CollectProcessingSessions10
 * @tc.desc: test CollectProcessingSessions with same ZOrder different isFocused
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CollectProcessingSessions10, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CollectProcessingSessions10";
    sessionInfo.abilityName_ = "CollectProcessingSessions10";

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession1->persistentId_ = 1;
    sceneSession1->SetZOrder(100);
    sceneSession1->isVisible_ = true;
    PostProcessFocusState state1 = { true, true, true, FocusChangeReason::FOREGROUND };
    sceneSession1->SetPostProcessFocusState(state1);

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession2->persistentId_ = 2;
    sceneSession2->SetZOrder(100);
    sceneSession2->isVisible_ = true;
    PostProcessFocusState state2 = { true, false, true, FocusChangeReason::DEFAULT };
    sceneSession2->SetPostProcessFocusState(state2);

    ssm_->sceneSessionMap_.emplace(1, sceneSession1);
    ssm_->sceneSessionMap_.emplace(2, sceneSession2);

    auto sessions = ssm_->CollectProcessingSessions();
    EXPECT_EQ(sessions.size(), 2);
    EXPECT_EQ(sessions[0]->GetPersistentId(), 1);

    ssm_->sceneSessionMap_.erase(1);
    ssm_->sceneSessionMap_.erase(2);
}

/**
 * @tc.name: CheckBlockingFocus_SystemSubWindow_Main
 * @tc.desc: SYSTEM_SUB_WINDOW inherits MAIN window blocking strategy
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CheckBlockingFocus_SystemSubWindow_Main, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo mainInfo;
    mainInfo.bundleName_ = "MainApp";
    sptr<SceneSession> mainSession = sptr<SceneSession>::MakeSptr(mainInfo, nullptr);
    mainSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    SessionInfo subInfo;
    subInfo.bundleName_ = "SubApp";
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(subInfo, nullptr);
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW);
    subSession->SetParentSession(mainSession);

    bool ret = ssm_->CheckBlockingFocus(subSession, true);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CheckBlockingFocus_SystemSubWindow_Main_PC
 * @tc.desc: SYSTEM_SUB_WINDOW with MAIN ancestor on PC still blocks
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CheckBlockingFocus_SystemSubWindow_Main_PC, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    SessionInfo mainInfo;
    mainInfo.bundleName_ = "MainApp";
    sptr<SceneSession> mainSession = sptr<SceneSession>::MakeSptr(mainInfo, nullptr);
    mainSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    SessionInfo subInfo;
    subInfo.bundleName_ = "SubApp";
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(subInfo, nullptr);
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW);
    subSession->SetParentSession(mainSession);

    bool ret = ssm_->CheckBlockingFocus(subSession, true);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CheckBlockingFocus_SystemSubWindow_Float_PC
 * @tc.desc: SYSTEM_SUB_WINDOW inherits FLOAT window strategy on PC (not blocking)
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CheckBlockingFocus_SystemSubWindow_Float_PC, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    SessionInfo mainInfo;
    mainInfo.bundleName_ = "MainApp";
    sptr<SceneSession> mainSession = sptr<SceneSession>::MakeSptr(mainInfo, nullptr);
    mainSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    SessionInfo floatInfo;
    floatInfo.bundleName_ = "FloatApp";
    sptr<SceneSession> floatSession = sptr<SceneSession>::MakeSptr(floatInfo, nullptr);
    floatSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    floatSession->SetParentSession(mainSession);

    SessionInfo subInfo;
    subInfo.bundleName_ = "SubApp";
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(subInfo, nullptr);
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW);
    subSession->SetParentSession(floatSession);

    bool ret = ssm_->CheckBlockingFocus(subSession, false);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CheckBlockingFocus_SystemSubWindow_NoAncestor
 * @tc.desc: SYSTEM_SUB_WINDOW with no ancestor returns false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CheckBlockingFocus_SystemSubWindow_NoAncestor, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo subInfo;
    subInfo.bundleName_ = "SubApp";
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(subInfo, nullptr);
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW);

    bool ret = ssm_->CheckBlockingFocus(subSession, false);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CheckBlockingFocus_NonAppSession
 * @tc.desc: Non-AppSession with includingAppSession=true returns false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CheckBlockingFocus_NonAppSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.bundleName_ = "SystemApp";
    info.isSystem_ = false;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->property_->SetWindowType(WindowType::WINDOW_TYPE_PANEL);

    bool ret = ssm_->CheckBlockingFocus(session, true);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CheckBlockingFocus_SpecialWindow_PC
 * @tc.desc: Special window types on PC do not block (only Phone/Pad)
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CheckBlockingFocus_SpecialWindow_PC, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;

    SessionInfo info;
    info.bundleName_ = "VoiceApp";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->property_->SetWindowType(WindowType::WINDOW_TYPE_VOICE_INTERACTION);

    bool ret = ssm_->CheckBlockingFocus(session, false);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CheckBlockingFocus_Float_PadFreeMultiWindow
 * @tc.desc: FLOAT window on Pad with free multi window mode does not block
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CheckBlockingFocus_Float_PadFreeMultiWindow, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ssm_->systemConfig_.freeMultiWindowEnable_ = true;
    ssm_->systemConfig_.freeMultiWindowSupport_ = true;

    SessionInfo mainInfo;
    mainInfo.bundleName_ = "MainApp";
    sptr<SceneSession> mainSession = sptr<SceneSession>::MakeSptr(mainInfo, nullptr);
    mainSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    SessionInfo floatInfo;
    floatInfo.bundleName_ = "FloatApp";
    sptr<SceneSession> floatSession = sptr<SceneSession>::MakeSptr(floatInfo, nullptr);
    floatSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    floatSession->SetParentSession(mainSession);

    bool ret = ssm_->CheckBlockingFocus(floatSession, false);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CheckBlockingFocus_NullSession
 * @tc.desc: Null session returns false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CheckBlockingFocus_NullSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    bool ret = ssm_->CheckBlockingFocus(nullptr, true);
    EXPECT_EQ(ret, false);

    ret = ssm_->CheckBlockingFocus(nullptr, false);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CheckBlockingFocus_SystemNoBlockingFocus
 * @tc.desc: System window without blockingFocus flag returns false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CheckBlockingFocus_SystemNoBlockingFocus, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.bundleName_ = "SystemApp";
    info.isSystem_ = true;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->property_->SetWindowType(WindowType::WINDOW_TYPE_PANEL);
    session->blockingFocus_ = false;

    bool ret = ssm_->CheckBlockingFocus(session, true);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CheckBlockingFocus_AppSession_IncludingFalse
 * @tc.desc: AppSession with includingAppSession=false returns false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerFocusTest, CheckBlockingFocus_AppSession_IncludingFalse, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;

    SessionInfo info;
    info.bundleName_ = "TestApp";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);

    bool ret = ssm_->CheckBlockingFocus(session, false);
    EXPECT_EQ(ret, false);
}
} // namespace
} // namespace Rosen
} // namespace OHOS