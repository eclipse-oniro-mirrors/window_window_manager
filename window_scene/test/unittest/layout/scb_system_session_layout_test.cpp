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

#include "common/include/session_permission.h"
#include "key_event.h"
#include "mock/mock_session_stage.h"
#include "session/host/include/scb_system_session.h"
#include "session/host/include/session.h"
#include "window_helper.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class SCBSystemSessionLayoutTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<SCBSystemSession::SpecificSessionCallback> specificCallback_ = nullptr;
    sptr<SCBSystemSession> scbSystemSession_;
};

void SCBSystemSessionLayoutTest::SetUpTestCase() {}

void SCBSystemSessionLayoutTest::TearDownTestCase() {}

void SCBSystemSessionLayoutTest::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testSCBSystemSession1";
    info.moduleName_ = "testSCBSystemSession2";
    info.bundleName_ = "testSCBSystemSession3";
    scbSystemSession_ = sptr<SCBSystemSession>::MakeSptr(info, specificCallback_);
}

void SCBSystemSessionLayoutTest::TearDown()
{
    scbSystemSession_ = nullptr;
}

namespace {
/**
 * @tc.name: UpdateWindowMode
 * @tc.desc: check func UpdateWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionLayoutTest, UpdateWindowMode, TestSize.Level1)
{
    scbSystemSession_->PresentFocusIfPointDown();
    scbSystemSession_->PresentFocusIfNeed(2, 0);
    ASSERT_EQ(WSError::WS_OK, scbSystemSession_->SetSystemSceneBlockingFocus(true));
    WSRect rect = { 0, 0, 0, 0 };
    scbSystemSession_->UpdatePointerArea(rect);
    auto ret = scbSystemSession_->UpdateWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_SESSION, ret);
}

/**
 * @tc.name: NotifyClientToUpdateRect03
 * @tc.desc: check func NotifyClientToUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionLayoutTest, NotifyClientToUpdateRect03, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);

    auto ret = scbSystemSession_->SetSessionProperty(property);
    ASSERT_EQ(WSError::WS_OK, ret);
    scbSystemSession_->isKeyboardPanelEnabled_ = true;
    ret = scbSystemSession_->NotifyClientToUpdateRect("SCBSystemSessionLayoutTest", std::nullopt, nullptr);
    ASSERT_EQ(WSError::WS_OK, ret);

    scbSystemSession_->isKeyboardPanelEnabled_ = false;
    ret = scbSystemSession_->NotifyClientToUpdateRect("SCBSystemSessionLayoutTest", std::nullopt, nullptr);
    ASSERT_EQ(WSError::WS_OK, ret);

    scbSystemSession_->isKeyboardPanelEnabled_ = true;
    ret = scbSystemSession_->NotifyClientToUpdateRect("SCBSystemSessionLayoutTest", std::nullopt, nullptr);
    ASSERT_EQ(WSError::WS_OK, ret);

    scbSystemSession_->isKeyboardPanelEnabled_ = false;
    ret = scbSystemSession_->NotifyClientToUpdateRect("SCBSystemSessionLayoutTest", std::nullopt, nullptr);
    ASSERT_EQ(WSError::WS_OK, ret);

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = scbSystemSession_->SetSessionProperty(property);
    ASSERT_EQ(WSError::WS_OK, ret);
    scbSystemSession_->isKeyboardPanelEnabled_ = true;
    ret = scbSystemSession_->NotifyClientToUpdateRect("SCBSystemSessionLayoutTest", std::nullopt, nullptr);
    ASSERT_EQ(WSError::WS_OK, ret);

    scbSystemSession_->isKeyboardPanelEnabled_ = false;
    ret = scbSystemSession_->NotifyClientToUpdateRect("SCBSystemSessionLayoutTest", std::nullopt, nullptr);
    ASSERT_EQ(WSError::WS_OK, ret);

    scbSystemSession_->isKeyboardPanelEnabled_ = true;
    ret = scbSystemSession_->NotifyClientToUpdateRect("SCBSystemSessionLayoutTest", std::nullopt, nullptr);
    ASSERT_EQ(WSError::WS_OK, ret);

    scbSystemSession_->isKeyboardPanelEnabled_ = false;
    ret = scbSystemSession_->NotifyClientToUpdateRect("SCBSystemSessionLayoutTest", std::nullopt, nullptr);
    ASSERT_EQ(WSError::WS_OK, ret);

    scbSystemSession_->Session::UpdateSizeChangeReason(SizeChangeReason::DRAG);
    ret = scbSystemSession_->NotifyClientToUpdateRect("SCBSystemSessionLayoutTest", std::nullopt, nullptr);
    ASSERT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: NotifyClientToUpdateRect04
 * @tc.desc: check func NotifyClientToUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(SCBSystemSessionLayoutTest, NotifyClientToUpdateRect04, TestSize.Level1)
{
    scbSystemSession_->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    auto ret = scbSystemSession_->NotifyClientToUpdateRect("SCBSystemSessionLayoutTest", std::nullopt, nullptr);
    ASSERT_EQ(WSError::WS_OK, ret);
}
} // namespace
} // namespace Rosen
} // namespace OHOS