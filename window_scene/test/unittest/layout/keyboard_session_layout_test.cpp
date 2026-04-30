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
#include "mock/mock_keyboard_session.h"
#include "mock/mock_session_stage.h"
#include "session/host/include/keyboard_session.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/session.h"
#include "window_helper.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class KeyboardSessionLayoutTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    sptr<SceneSession> GetSceneSession(const std::string& abilityName, const std::string& bundleName);
};

void KeyboardSessionLayoutTest::SetUpTestCase() {}

void KeyboardSessionLayoutTest::TearDownTestCase() {}

void KeyboardSessionLayoutTest::SetUp() {}

void KeyboardSessionLayoutTest::TearDown() {}

sptr<SceneSession> KeyboardSessionLayoutTest::GetSceneSession(const std::string& abilityName,
                                                              const std::string& bundleName)
{
    SessionInfo info;
    info.abilityName_ = abilityName;
    info.bundleName_ = bundleName;
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    return sptr<SceneSession>::MakeSptr(info, specificCb);
}

namespace {
/**
 * @tc.name: NotifyClientToUpdateRect01
 * @tc.desc: NotifyClientToUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionLayoutTest, NotifyClientToUpdateRect01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyClientToUpdateRect01";
    info.bundleName_ = "NotifyClientToUpdateRect01";
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    auto keyboardCb = sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    keyboardSession->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
    keyboardSession->sessionStage_ = mockSessionStage;
    auto ret = keyboardSession->NotifyClientToUpdateRect("KeyboardSessionLayoutTest", std::nullopt, nullptr);
    ASSERT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: NotifyClientToUpdateRect02
 * @tc.desc: NotifyClientToUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionLayoutTest, NotifyClientToUpdateRect02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyClientToUpdateRect02";
    info.bundleName_ = "NotifyClientToUpdateRect02";
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    auto keyboardCb = sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    keyboardSession->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
    keyboardSession->Session::UpdateSizeChangeReason(SizeChangeReason::MOVE);
    keyboardSession->isKeyboardPanelEnabled_ = true;
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    windowSessionProperty->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    keyboardSession->SetSessionProperty(windowSessionProperty);
    auto ret = keyboardSession->NotifyClientToUpdateRect("KeyboardSessionLayoutTest", std::nullopt, nullptr);
    ASSERT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: NotifyClientToUpdateRect03
 * @tc.desc: NotifyClientToUpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionLayoutTest, NotifyClientToUpdateRect03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyClientToUpdateRect03";
    info.bundleName_ = "NotifyClientToUpdateRect03";
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    auto keyboardCb = sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);

    // NotifyClientToUpdateRectTask return not ok
    WSError ret = keyboardSession->NotifyClientToUpdateRect("KeyboardSessionLayoutTest", std::nullopt, nullptr);
    ASSERT_EQ(ret, WSError::WS_OK);

    // NotifyClientToUpdateRectTask return ok and GetSizeChangeReason() is UNDEFINED
    keyboardSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    keyboardSession->state_ = SessionState::STATE_CONNECT;
    ret = keyboardSession->NotifyClientToUpdateRect("KeyboardSessionLayoutTest", std::nullopt, nullptr);
    ASSERT_EQ(ret, WSError::WS_OK);

    // NotifyClientToUpdateRectTask return ok and GetSizeChangeReason() is DRAG
    keyboardSession->Session::UpdateSizeChangeReason(SizeChangeReason::DRAG);
    keyboardSession->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
    ret = keyboardSession->NotifyClientToUpdateRect("KeyboardSessionLayoutTest", std::nullopt, nullptr);
    ASSERT_EQ(ret, WSError::WS_OK);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
