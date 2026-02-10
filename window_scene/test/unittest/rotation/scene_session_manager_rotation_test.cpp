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
#include "mock_scene_session.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "session_manager.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class SceneSessionManagerAnimationTest : public testing::Test {
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

sptr<SceneSessionManager> SceneSessionManagerAnimationTest::ssm_ = nullptr;

void NotifyRecoverSceneSessionFuncTest(const sptr<SceneSession>& session, const SessionInfo& sessionInfo) {}

bool TraverseFuncTest(const sptr<SceneSession>& session)
{
    return true;
}

void WindowChangedFuncTest(int32_t persistentId, WindowUpdateType type) {}

void ProcessStatusBarEnabledChangeFuncTest(bool enable) {}

void SceneSessionManagerAnimationTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerAnimationTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerAnimationTest::SetUp() {}

void SceneSessionManagerAnimationTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
}

void SceneSessionManagerAnimationTest::InitTestSceneSession(DisplayId displayId,
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

void SceneSessionManagerAnimationTest::InitTestSceneSessionForListWindowInfo()
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
    sessionInfo4.isSystem_ = true;
    sptr<SceneSession> sceneSession6 = sptr<SceneSession>::MakeSptr(sessionInfo4, nullptr);
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
 * @tc.name: TestUpdateRotateAnimationConfig_01
 * @tc.desc: Test UpdateRotateAnimationConfig with duration_ 400
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAnimationTest, TestUpdateRotateAnimationConfig_01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    RotateAnimationConfig config = { 400 };
    ssm_->UpdateRotateAnimationConfig(config);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(ssm_->rotateAnimationConfig_.duration_, 400);
}

/**
 * @tc.name: TestUpdateRotateAnimationConfig_02
 * @tc.desc: Test UpdateRotateAnimationConfig with duration_ 600
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAnimationTest, TestUpdateRotateAnimationConfig_02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    RotateAnimationConfig config = { 600 };
    ssm_->UpdateRotateAnimationConfig(config);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(ssm_->rotateAnimationConfig_.duration_, 600);
}

/**
 * @tc.name: GetActiveSceneSessionCopy
 * @tc.desc: test function : GetActiveSceneSessionCopy
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerAnimationTest, GetActiveSceneSessionCopy, Function | SmallTest | Level2)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();

    SessionInfo info;
    info.abilityName_ = "GetActiveSceneSessionCopy";
    info.bundleName_ = "GetActiveSceneSessionCopy";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSessionMocker> sceneSession = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    std::vector<sptr<SceneSession>> activeSession = ssm_->GetActiveSceneSessionCopy();
    EXPECT_EQ(activeSession.empty(), false);

    ssm_->sceneSessionMap_.clear();
}
} // namespace
} // namespace Rosen
} // namespace OHOS