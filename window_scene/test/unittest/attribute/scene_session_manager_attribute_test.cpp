/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <bundle_mgr_interface.h>
#include <bundlemgr/launcher_service.h>
#include <gtest/gtest.h>
#include <regex>

#include "context.h"
#include "interfaces/include/ws_common.h"
#include "mock/mock_accesstoken_kit.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "mock/mock_ibundle_mgr.h"
#include "pointer_event.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/main_session.h"
#include "session_info.h"
#include "session_manager.h"
#include "session_manager/include/scene_session_manager.h"
#include "window_manager_agent.h"
#include "window_manager_hilog.h"
#include "zidl/window_manager_agent_interface.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SceneSessionManagerAttributeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t waitSyncInNs = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerAttributeTest::ssm_ = nullptr;

void SceneSessionManagerAttributeTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerAttributeTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerAttributeTest::SetUp()
{
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerAttributeTest::TearDown()
{
    MockAccesstokenKit::ChangeMockStateToInit();
    usleep(waitSyncInNs);
    ssm_->sceneSessionMap_.clear();
}

namespace {
/**
 * @tc.name: GetTopNavDestinationName
 * @tc.desc: test GetTopNavDestinationName whether get the top nav destination name.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, GetTopNavDestinationName, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, ssm_);
    auto oldSceneSessionMap = ssm_->sceneSessionMap_;
    ssm_->sceneSessionMap_.clear();

    std::string topNavDestName;
    EXPECT_EQ(ssm_->GetTopNavDestinationName(1000, topNavDestName), WMError::WM_ERROR_INVALID_WINDOW);

    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession));
    EXPECT_EQ(ssm_->GetTopNavDestinationName(2, topNavDestName), WMError::WM_ERROR_INVALID_OPERATION);

    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->sessionStage_ = nullptr;
    EXPECT_EQ(ssm_->GetTopNavDestinationName(2, topNavDestName), WMError::WM_ERROR_SYSTEM_ABNORMALLY);

    sceneSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    EXPECT_EQ(ssm_->GetTopNavDestinationName(2, topNavDestName), WMError::WM_OK);
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_ = oldSceneSessionMap;
}

/**
 * @tc.name: IsNeedNotifyScreenshotEvent
 * @tc.desc: test IsNeedNotifyScreenshotEvent.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAttributeTest, IsNeedNotifyScreenshotEvent, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto oldSceneSessionMap = ssm_->sceneSessionMap_;
    auto oldScreenshotEventListenerSessionSet = ssm_->screenshotAppEventListenerSessionSet_;
    auto oldScreenshotListenerSessionSet = ssm_->screenshotListenerSessionSet_;
    ssm_->screenshotListenerSessionSet_.clear();
    ssm_->sceneSessionMap_.clear();
    ssm_->screenshotAppEventListenerSessionSet_.clear();
    EXPECT_EQ(ssm_->IsNeedNotifyScreenshotEvent(nullptr), false);

    SessionInfo sessionInfo;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->property_->SetPersistentId(100);
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession->GetPersistentId(), sceneSession));
    sceneSession->SetSessionState(SessionState::STATE_DISCONNECT);
    EXPECT_EQ(ssm_->IsNeedNotifyScreenshotEvent(sceneSession), false);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    EXPECT_EQ(ssm_->IsNeedNotifyScreenshotEvent(sceneSession), true);

    SessionInfo sessionInfo2;
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    subSession->property_->SetPersistentId(101);
    subSession->property_->SetParentPersistentId(sceneSession->GetPersistentId());
    subSession->SetParentSession(sceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(subSession->GetPersistentId(), subSession));
    ssm_->screenshotAppEventListenerSessionSet_.insert(subSession->GetPersistentId());

    subSession->SetSessionState(SessionState::STATE_DISCONNECT);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    EXPECT_EQ(ssm_->IsNeedNotifyScreenshotEvent(subSession), true);
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    EXPECT_NE(ssm_->UpdateSessionScreenshotListener(1100, false), WMError::WM_OK);
    EXPECT_EQ(ssm_->UpdateSessionScreenshotListener(sceneSession->GetPersistentId(), true), WMError::WM_OK);
    ssm_->OnScreenshot(0);
    auto ret = ssm_->NotifyScreenshotEvent(ScreenshotEventType::SCROLL_SHOT_START);
    EXPECT_NE(ret, WMError::WM_ERROR_INVALID_CALLING);
    EXPECT_EQ(ssm_->IsNeedNotifyScreenshotEvent(subSession), true);
    EXPECT_EQ(ssm_->UpdateSessionScreenshotListener(sceneSession->GetPersistentId(), false), WMError::WM_OK);

    ssm_->sceneSessionMap_.clear();
    ssm_->screenshotAppEventListenerSessionSet_.clear();
    ssm_->screenshotListenerSessionSet_.clear();
    ssm_->screenshotListenerSessionSet_ = oldScreenshotListenerSessionSet;
    ssm_->sceneSessionMap_ = oldSceneSessionMap;
    ssm_->screenshotAppEventListenerSessionSet_ = oldScreenshotEventListenerSessionSet;
}
} // namespace
} // namespace Rosen
} // namespace OHOS
