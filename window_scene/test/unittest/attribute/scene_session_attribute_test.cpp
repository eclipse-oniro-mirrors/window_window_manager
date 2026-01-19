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

#include "window_helper.h"
#include "display_manager.h"
#include "pointer_event.h"

#include <gtest/gtest.h>
#include "key_event.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/sub_session.h"
#include "session/host/include/system_session.h"
#include "session/host/include/main_session.h"
#include "wm_common.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_accesstoken_kit.h"
#include "input_event.h"
#include <pointer_event.h>
#include <ui/rs_surface_node.h>

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
class SceneSessionAttributeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneSessionAttributeTest::SetUpTestCase() {}

void SceneSessionAttributeTest::TearDownTestCase() {}

void SceneSessionAttributeTest::SetUp() {}

void SceneSessionAttributeTest::TearDown() {}

namespace {
/**
 * @tc.name: SetWindowShadowEnabled01
 * @tc.desc: SetWindowShadowEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionAttributeTest, SetWindowShadowEnabled01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWindowShadowEnabled";
    info.bundleName_ = "SetWindowShadowEnabled";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    bool isEnabled = true;
    EXPECT_EQ(WSError::WS_OK, sceneSession->SetWindowShadowEnabled(isEnabled));
    EXPECT_EQ(isEnabled, sceneSession->GetWindowShadowEnabled());
    isEnabled = false;
    EXPECT_EQ(WSError::WS_OK, sceneSession->SetWindowShadowEnabled(isEnabled));
    EXPECT_EQ(isEnabled, sceneSession->GetWindowShadowEnabled());
}
} // namespace
} // namespace Rosen
} // namespace OHOS