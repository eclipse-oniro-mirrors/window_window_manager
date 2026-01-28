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
} // namespace
} // namespace Rosen
} // namespace OHOS