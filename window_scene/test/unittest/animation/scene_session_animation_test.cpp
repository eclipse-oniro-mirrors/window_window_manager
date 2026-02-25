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

#include "application_context.h"
#include "input_event.h"
#include "key_event.h"
#include "mock/mock_session_stage.h"
#include "pointer_event.h"

#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session/host/include/main_session.h"
#include "session/host/include/keyboard_session.h"
#define PRIVATE public
#define PROTECTED public
#include "session/host/include/scene_session.h"
#undef PRIVATE
#undef PROTECTED
#include "session/host/include/sub_session.h"
#include "session/host/include/system_session.h"
#include "ui/rs_surface_node.h"
#include "window_helper.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class SceneSessionAnimationTest : public testing::Test {
public:
    SceneSessionAnimationTest();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    sptr<ScreenSession> defaultScreenSession_;
    ScreenSessionManagerClient& ssmClient_;
};

SceneSessionAnimationTest::SceneSessionAnimationTest() : ssmClient_(ScreenSessionManagerClient::GetInstance())
{
    constexpr ScreenId defaultScreenId = 1001;
    auto screenProperty = ScreenProperty();
    defaultScreenSession_ = sptr<ScreenSession>::MakeSptr(defaultScreenId, screenProperty, defaultScreenId);
    {
        std::lock_guard<std::mutex> lock(ssmClient_.screenSessionMapMutex_);
        ssmClient_.screenSessionMap_[defaultScreenId] = defaultScreenSession_;
    }
}

void SceneSessionAnimationTest::SetUpTestCase() {}

void SceneSessionAnimationTest::TearDownTestCase()
{
}

void SceneSessionAnimationTest::SetUp() {}

void SceneSessionAnimationTest::TearDown() {}

namespace {
/**
 * @tc.name: SetWindowCornerRadiusCallback
 * @tc.desc: SetWindowCornerRadiusCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionAnimationTest, SetWindowCornerRadiusCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWindowCornerRadiusCallback";
    info.bundleName_ = "SetWindowCornerRadiusCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    NotifySetWindowCornerRadiusFunc func1 = [](float cornerRadius) { return; };
    sceneSession->SetWindowCornerRadiusCallback(std::move(func1));
    ASSERT_NE(nullptr, sceneSession->onSetWindowCornerRadiusFunc_);
    ASSERT_NE(nullptr, sceneSession->property_);
    EXPECT_EQ(WINDOW_CORNER_RADIUS_INVALID, sceneSession->property_->GetWindowCornerRadius());
}

/**
 * @tc.name: SetWindowCornerRadiusCallback01
 * @tc.desc: SetWindowCornerRadiusCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionAnimationTest, SetWindowCornerRadiusCallback01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWindowCornerRadiusCallback01";
    info.bundleName_ = "SetWindowCornerRadiusCallback01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    NotifySetWindowCornerRadiusFunc func1 = [](float cornerRadius) { return; };
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowCornerRadius(1); // 1 is valid window corner radius
    sceneSession->SetWindowCornerRadiusCallback(std::move(func1));
    ASSERT_NE(nullptr, sceneSession->onSetWindowCornerRadiusFunc_);
    EXPECT_EQ(1, sceneSession->property_->GetWindowCornerRadius()); // 1 is valid window corner radius
}

/**
 * @tc.name: OnSetWindowCornerRadius
 * @tc.desc: OnSetWindowCornerRadius
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionAnimationTest, SetWindowCornerRadius, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWindowCornerRadius";
    info.bundleName_ = "SetWindowCornerRadius";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);
    EXPECT_EQ(WSError::WS_OK, session->SetWindowCornerRadius(1.0f));

    NotifySetWindowCornerRadiusFunc func = [](float cornerRadius) { return; };
    session->onSetWindowCornerRadiusFunc_ = func;
    EXPECT_EQ(WSError::WS_OK, session->SetWindowCornerRadius(1.0f));
}

/**
 * @tc.name: SetWindowShadowsCallback
 * @tc.desc: SetWindowShadowsCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionAnimationTest, SetWindowShadowsCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWindowShadowsCallback";
    info.bundleName_ = "SetWindowShadowsCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    NotifySetWindowShadowsFunc func = [](ShadowsInfo shadowsInfo) {
        return;
    };
    sceneSession->SetWindowShadowsCallback(std::move(func));
    ASSERT_NE(nullptr, sceneSession->onSetWindowShadowsFunc_);
}

/**
 * @tc.name: OnSetWindowShadows
 * @tc.desc: OnSetWindowShadows
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionAnimationTest, OnSetWindowShadows, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnSetWindowShadows";
    info.bundleName_ = "OnSetWindowShadows";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(session, nullptr);

    ShadowsInfo shadowsInfo = { 20.0, "#FF0000", 0.0, 0.0, true, true, true, true };
    EXPECT_EQ(WSError::WS_OK, session->SetWindowShadows(shadowsInfo));

    NotifySetWindowShadowsFunc func = [](ShadowsInfo shadowsInfo) {
        return;
    };
    session->onSetWindowShadowsFunc_ = func;
    EXPECT_EQ(WSError::WS_OK, session->SetWindowShadows(shadowsInfo));
}
} // namespace
} // namespace Rosen
} // namespace OHOS