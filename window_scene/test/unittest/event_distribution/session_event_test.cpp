/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <regex>
#include <pointer_event.h>
#include <ui/rs_surface_node.h>

#include "session/host/include/extension_session.h"
#include "session/host/include/move_drag_controller.h"
#include "session/host/include/scene_session.h"
#include "session_manager/include/scene_session_manager.h"
#include "session/host/include/pc_fold_screen_manager.h"
#include "session/host/include/session.h"
#include "session_info.h"
#include "key_event.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SessionEventTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<Session> session_ = nullptr;
};

void SessionEventTest::SetUpTestCase() {}

void SessionEventTest::TearDownTestCase() {}

void SessionEventTest::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession3";
    session_ = sptr<Session>::MakeSptr(info);
}

void SessionEventTest::TearDown()
{
    session_ = nullptr;
}

namespace {
/**
 * @tc.name: SetTouchable01
 * @tc.desc: IsSessionValid() return false
 * @tc.type: FUNC
 */
HWTEST_F(SessionEventTest, SetTouchable01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_DISCONNECT;
    session_->sessionInfo_.isSystem_ = true;
    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, session_->SetTouchable(false));
}

/**
 * @tc.name: SetTouchable02
 * @tc.desc: IsSessionValid() return true
 * @tc.type: FUNC
 */
HWTEST_F(SessionEventTest, SetTouchable02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_FOREGROUND;
    session_->sessionInfo_.isSystem_ = false;
    ASSERT_EQ(WSError::WS_OK, session_->SetTouchable(false));
}

/**
 * @tc.name: SetTouchable03
 * @tc.desc: IsSessionValid() and touchable return true
 * @tc.type: FUNC
 */
HWTEST_F(SessionEventTest, SetTouchable03, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetSessionState(SessionState::STATE_FOREGROUND);
    session_->sessionInfo_.isSystem_ = false;
    EXPECT_EQ(WSError::WS_OK, session_->SetTouchable(true));
}
}
}
}