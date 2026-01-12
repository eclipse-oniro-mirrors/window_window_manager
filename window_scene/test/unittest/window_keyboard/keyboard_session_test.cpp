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

#include "session/host/include/keyboard_session.h"
#include <gtest/gtest.h>

#include "interfaces/include/ws_common.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_keyboard_session.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session/host/include/session.h"
#include "session/host/include/scene_session.h"
#include "ui/rs_surface_node.h"
#include "window_helper.h"
#include "window_manager_hilog.h"
#include "window_session_property.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "KeyboardSessionTest" };
}

class KeyboardSessionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    sptr<SceneSession> GetSceneSession(const std::string& abilityName, const std::string& bundleName);
    static constexpr uint32_t SPLIT_TEST_SLEEP_S = 1;
};

void KeyboardSessionTest::SetUpTestCase() {}

void KeyboardSessionTest::TearDownTestCase() {}

void KeyboardSessionTest::SetUp() {}

void KeyboardSessionTest::TearDown() {}

sptr<SceneSession> KeyboardSessionTest::GetSceneSession(const std::string& abilityName, const std::string& bundleName)
{
    SessionInfo info;
    info.abilityName_ = abilityName;
    info.bundleName_ = bundleName;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);

    return sceneSession;
}

namespace {
/**
 * @tc.name: GetKeyboardGravity
 * @tc.desc: test function: GetKeyboardGravity
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, GetKeyboardGravity, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetKeyboardGravity";
    info.bundleName_ = "GetKeyboardGravity";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    keyboardSession->property_ = windowSessionProperty;
    ASSERT_EQ(SessionGravity::SESSION_GRAVITY_BOTTOM, keyboardSession->GetKeyboardGravity());
}

/**
 * @tc.name: Show01
 * @tc.desc: test function: Show
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, Show01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "Show01";
    info.bundleName_ = "Show01";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    EXPECT_NE(keyboardSession, nullptr);

    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, keyboardSession->Show(nullptr));

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_EQ(WSError::WS_OK, keyboardSession->Show(property));

    ASSERT_NE(nullptr, keyboardSession->property_);
    KeyboardLayoutParams params;
    keyboardSession->property_->SetKeyboardLayoutParams(params);
    ASSERT_EQ(SessionGravity::SESSION_GRAVITY_BOTTOM, keyboardSession->GetKeyboardGravity());
    ASSERT_EQ(WSError::WS_OK, keyboardSession->Show(property));
}

/**
 * @tc.name: Show02
 * @tc.desc: test function: Show
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, Show02, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "Show02";
    info.bundleName_ = "Show02";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ASSERT_EQ(WSError::WS_OK, keyboardSession->Show(property));

    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_EQ(WSError::WS_OK, keyboardSession->Show(property));

    keyboardSession->SetSurfaceNode(nullptr);
    ASSERT_EQ(WSError::WS_OK, keyboardSession->Show(property));

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    ASSERT_NE(surfaceNode, nullptr);
    keyboardSession->SetSurfaceNode(surfaceNode);
    ASSERT_EQ(WSError::WS_OK, keyboardSession->Show(property));
}

/**
 * @tc.name: Hide
 * @tc.desc: test function: Hide
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, Hide, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "Hide";
    info.bundleName_ = "Hide";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    ASSERT_TRUE((keyboardSession != nullptr));

    ASSERT_EQ(WSError::WS_OK, keyboardSession->Hide());
}

/**
 * @tc.name: Disconnect
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, Disconnect, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "Disconnect";
    info.bundleName_ = "Disconnect";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->SetSessionProperty(property);
    keyboardSession->isActive_ = true;
    auto result = keyboardSession->Disconnect();
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: Disconnect01
 * @tc.desc: test system keyboard disconnect
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, DisConnect01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "DisConnect01";
    info.bundleName_ = "DisConnect01";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    ASSERT_EQ(WSError::WS_OK, keyboardSession->SetSessionProperty(property));
    keyboardSession->isActive_ = true;
    ASSERT_EQ(WSError::WS_OK, keyboardSession->Disconnect());

    keyboardSession->SetIsSystemKeyboard(true);
    ASSERT_EQ(WSError::WS_OK, keyboardSession->Disconnect());
}

/**
 * @tc.name: Disconnect02
 * @tc.desc: test when keybaordPanelSession_ is null
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, DisConnect02, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "DisConnect02";
    info.bundleName_ = "DisConnect02";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    ASSERT_EQ(WSError::WS_OK, keyboardSession->SetSessionProperty(property));
    keyboardSession->isActive_ = true;
    keyboardSession->keyboardPanelSession_ = nullptr;
    ASSERT_EQ(WSError::WS_OK, keyboardSession->Disconnect());

    SessionInfo info_panel;
    info_panel.abilityName_ = "BindKeyboardPanelSession";
    info_panel.bundleName_ = "BindKeyboardPanelSession";
    sptr<SceneSession> panelSession = sptr<SceneSession>::MakeSptr(info_panel, nullptr);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    keyboardSession->keyboardPanelSession_ = panelSession;
    ASSERT_EQ(WSError::WS_OK, keyboardSession->Disconnect());
}

/**
 * @tc.name: GetSceneSession01
 * @tc.desc: GetSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, GetSceneSession01, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "GetSceneSession01";
    info.bundleName_ = "GetSceneSession01";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    EXPECT_NE(keyboardSession, nullptr);
    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    EXPECT_NE(sceneSession, nullptr);
    auto id = sceneSession->GetPersistentId();
    EXPECT_NE(id, 0);
    auto ret = keyboardSession->GetSceneSession(id);

    keyboardCb->onGetSceneSession = [](uint32_t) { return nullptr; };
    EXPECT_NE(keyboardCb->onGetSceneSession, nullptr);
    ret = keyboardSession->GetSceneSession(id);
}

/**
 * @tc.name: NotifyRootSceneOccupiedAreaChange
 * @tc.desc: NotifyRootSceneOccupiedAreaChange
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, NotifyRootSceneOccupiedAreaChange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRootSceneOccupiedAreaChange";
    info.bundleName_ = "NotifyRootSceneOccupiedAreaChange";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    EXPECT_NE(keyboardSession, nullptr);
    auto occupiedInfo = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    ASSERT_NE(nullptr, occupiedInfo);
    keyboardSession->NotifyRootSceneOccupiedAreaChange(occupiedInfo);
    keyboardSession->GetSessionProperty()->SetDisplayId(2025);
    keyboardSession->NotifyRootSceneOccupiedAreaChange(occupiedInfo);
    keyboardSession->GetSessionProperty()->SetDisplayId(0);
    keyboardSession->keyboardCallback_->onNotifyOccupiedAreaChange = nullptr;
    keyboardSession->NotifyRootSceneOccupiedAreaChange(occupiedInfo);
    keyboardSession->keyboardCallback_ = nullptr;
    keyboardSession->NotifyRootSceneOccupiedAreaChange(occupiedInfo);
}

/**
 * @tc.name: NotifyRootSceneOccupiedAreaChange02
 * @tc.desc: NotifyRootSceneOccupiedAreaChange
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, NotifyRootSceneOccupiedAreaChange02, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "NotifyRootSceneOccupiedAreaChange02";
    info.bundleName_ = "NotifyRootSceneOccupiedAreaChange02";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ASSERT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    ASSERT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    ASSERT_NE(keyboardSession, nullptr);
    auto occupiedInfo = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    ASSERT_NE(occupiedInfo, nullptr);
    auto ret = 1;
    keyboardSession->keyboardCallback_->onNotifyOccupiedAreaChange =
        [&ret](const sptr<OccupiedAreaChangeInfo>& info) -> void { ret += 1; };
    keyboardSession->GetSessionProperty()->SetDisplayId(ScreenSessionManagerClient::GetInstance().GetDefaultScreenId());
    keyboardSession->NotifyRootSceneOccupiedAreaChange(occupiedInfo);
    EXPECT_NE(ret, 1);
}

/**
 * @tc.name: RestoreCallingSession
 * @tc.desc: RestoreCallingSession
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, RestoreCallingSession, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "RestoreCallingSession";
    info.bundleName_ = "RestoreCallingSession";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    EXPECT_NE(keyboardSession, nullptr);

    // callingSession is nullptr
    keyboardSession->RestoreCallingSession(0, nullptr);

    // callingsession is not nullptr
    info.windowType_ = 1; // 1 is main_window_type
    sptr<SceneSession> callingSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    EXPECT_NE(callingSession, nullptr);
    ASSERT_NE(keyboardSession->keyboardCallback_, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession =
        [callingSession](int32_t persistentId) -> sptr<SceneSession> { return callingSession; };
    ASSERT_NE(callingSession->property_, nullptr);
    uint32_t callingId = callingSession->property_->GetPersistentId();
    keyboardSession->RestoreCallingSession(callingId, nullptr);
    ASSERT_EQ(callingSession->GetOriPosYBeforeRaisedByKeyboard(), 0); // 0: default value

    callingSession->SetOriPosYBeforeRaisedByKeyboard(100); // 100 is not default
    callingSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    keyboardSession->RestoreCallingSession(callingId, nullptr);
    ASSERT_EQ(callingSession->GetOriPosYBeforeRaisedByKeyboard(), 0); // 0: default value

    callingSession->SetOriPosYBeforeRaisedByKeyboard(100); // 100 is not default
    WSRect lastSafeRect = { 1, 2, 3, 4 };
    callingSession->SetLastSafeRect(lastSafeRect);
    callingSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    keyboardSession->RestoreCallingSession(callingId, nullptr);
    ASSERT_EQ(callingSession->GetOriPosYBeforeRaisedByKeyboard(), 0); // 0: default value
}

/**
 * @tc.name: RestoreCallingSession02
 * @tc.desc: RestoreCallingSession
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, RestoreCallingSession02, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "RestoreCallingSession02";
    info.bundleName_ = "RestoreCallingSession02";
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    auto keyboardCb = sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);

    // callingsession is not nullptr
    info.windowType_ = 1; // 1 is main_window_type
    sptr<SceneSession> callingSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    EXPECT_NE(callingSession, nullptr);
    ASSERT_NE(keyboardSession->keyboardCallback_, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession =
        [callingSession](int32_t persistentId) -> sptr<SceneSession> { return callingSession; };
    ASSERT_NE(callingSession->property_, nullptr);
    uint32_t callingId = callingSession->property_->GetPersistentId();
    keyboardSession->keyboardAvoidAreaActive_ = false;
    callingSession->SetOriPosYBeforeRaisedByKeyboard(100); // 100 is not default
    keyboardSession->RestoreCallingSession(callingId, nullptr);
    ASSERT_NE(callingSession->GetOriPosYBeforeRaisedByKeyboard(), 0); // 0: default value
}

/**
 * @tc.name: UseFocusIdIfCallingSessionIdInvalid
 * @tc.desc: UseFocusIdIfCallingSessionIdInvalid
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, UseFocusIdIfCallingSessionIdInvalid, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UseFocusIdIfCallingSessionIdInvalid";
    info.bundleName_ = "UseFocusIdIfCallingSessionIdInvalid";
    sptr<SceneSession::SpecificSessionCallback> specificCb =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    EXPECT_NE(keyboardSession, nullptr);

    info.windowType_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    EXPECT_NE(sceneSession, nullptr);
    auto id = sceneSession->GetPersistentId();
    EXPECT_NE(id, 0);

    keyboardSession->GetSessionProperty()->SetCallingSessionId(id);
    keyboardSession->UseFocusIdIfCallingSessionIdInvalid(id);
}

/**
 * @tc.name: GetFocusedSessionId
 * @tc.desc: GetFocusedSessionId
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, GetFocusedSessionId, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetFocusedSessionId";
    info.bundleName_ = "GetFocusedSessionId";
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    keyboardCb->onGetFocusedSessionId = []() { return 0; };
    EXPECT_NE(keyboardCb->onGetFocusedSessionId, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, keyboardCb);
    ASSERT_EQ(INVALID_WINDOW_ID, keyboardSession->GetFocusedSessionId());

    keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_EQ(INVALID_WINDOW_ID, keyboardSession->GetFocusedSessionId());
}

/**
 * @tc.name: SetCallingSessionId
 * @tc.desc: SetCallingSessionId
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, SetCallingSessionId, TestSize.Level0)
{
    WLOGFI("SetCallingSessionId begin!");
    SessionInfo info;
    info.abilityName_ = "SetCallingSessionId";
    info.bundleName_ = "SetCallingSessionId";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    EXPECT_NE(keyboardSession, nullptr);

    // keyboardCallback_->onGetSceneSession is nullptr, getCallingSession is nullptr
    keyboardSession->SetCallingSessionId(0);
    ASSERT_EQ(keyboardSession->GetCallingSessionId(), INVALID_SESSION_ID);

    // getCallingSession is not nullptr
    info.windowType_ = 1; // 1 is main_window_type
    sptr<SceneSession> callingSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    EXPECT_NE(callingSession, nullptr);
    ASSERT_NE(keyboardSession->keyboardCallback_, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession =
        [callingSession](int32_t persistenId) -> sptr<SceneSession> {
        if (persistenId != 100) { // callingSession's persistentId is 100
            return nullptr;
        }
        return callingSession;
    };
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() -> int32_t {
        return 100; // focusSession's persistentId is 100
    };
    keyboardSession->keyboardCallback_->onCallingSessionIdChange = [](int32_t callingSessionid) {};

    keyboardSession->SetCallingSessionId(0);
    ASSERT_EQ(keyboardSession->GetCallingSessionId(), 100); // 100 is callingSessionId
}

/**
 * @tc.name: SetCallingSessionId02
 * @tc.desc: SetCallingSessionId02
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, SetCallingSessionId02, TestSize.Level0)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "SetCallingSessionId02 begin!");
    SessionInfo info;
    info.abilityName_ = "SetCallingSessionId02";
    info.bundleName_ = "SetCallingSessionId02";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    EXPECT_NE(keyboardSession, nullptr);

    // getCallingSession is not nullptr
    info.windowType_ = 1; // 1 is main_window_type
    sptr<SceneSession> callingSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    EXPECT_NE(callingSession, nullptr);
    ASSERT_NE(keyboardSession->keyboardCallback_, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession =
        [callingSession](int32_t persistenId) -> sptr<SceneSession> {
        return callingSession;
    };
    keyboardSession->keyboardCallback_->onCallingSessionIdChange = [](int32_t callingSessionid) {};

    keyboardSession->state_ = SessionState::STATE_FOREGROUND;
    keyboardSession->SetCallingSessionId(50); // 50 is callingSessionId
    keyboardSession->SetCallingSessionId(100); // 100 is callingSessionId
    ASSERT_EQ(keyboardSession->GetCallingSessionId(), 100); // 100 is callingSessionId
}

/**
 * @tc.name: SetCallingSessionId03
 * @tc.desc: SetCallingSessionId03
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, SetCallingSessionId03, TestSize.Level0)
{
    TLOGI(WmsLogTag::WMS_KEYBOARD, "SetCallingSessionId03 begin!");
    SessionInfo info;
    info.abilityName_ = "SetCallingSessionId03";
    info.bundleName_ = "SetCallingSessionId03";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    EXPECT_NE(keyboardSession, nullptr);

    // getCallingSession is not nullptr
    info.windowType_ = 1; // 1 is main_window_type
    sptr<SceneSession> callingSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    EXPECT_NE(callingSession, nullptr);
    ASSERT_NE(keyboardSession->keyboardCallback_, nullptr);
    keyboardSession->keyboardCallback_->onGetSceneSession =
        [callingSession](int32_t persistenId) -> sptr<SceneSession> {
        if (persistenId != 100) { // callingSession's persistentId is 100
            return nullptr;
        }
        return callingSession;
    };
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() -> int32_t {
        return 100; // focusSession's persistentId is 100
    };

    keyboardSession->keyboardCallback_->onCallingSessionIdChange = nullptr;
    keyboardSession->SetCallingSessionId(100); // 100 is callingSessionId

    keyboardSession->keyboardCallback_->onCallingSessionIdChange = [](int32_t callingSessionid) {};
    keyboardSession->SetCallingSessionId(100); // 100 is callingSessionId
    ASSERT_EQ(keyboardSession->GetCallingSessionId(), 100); // 100 is callingSessionId
}

/**
 * @tc.name: GetCallingSessionId
 * @tc.desc: GetCallingSessionId
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, GetCallingSessionId, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "GetCallingSessionId";
    info.bundleName_ = "GetCallingSessionId";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    EXPECT_NE(keyboardSession, nullptr);
    auto ret = keyboardSession->GetCallingSessionId();
    ASSERT_EQ(ret, INVALID_WINDOW_ID);
}

/**
 * @tc.name: GetCallingSessionId01
 * @tc.desc: GetCallingSessionId01
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, GetCallingSessionId01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetCallingSessionId";
    info.bundleName_ = "GetCallingSessionId";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(keyboardSession, nullptr);
    auto ret = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(ret, INVALID_SESSION_ID);
    ASSERT_NE(keyboardSession->property_, nullptr);
    keyboardSession->property_->SetCallingSessionId(1);
    ret = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(ret, 1);
}

/**
 * @tc.name: NotifySystemKeyboardAvoidChange
 * @tc.desc: test NotifySystemKeyboardAvoidChange
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, NotifySystemKeyboardAvoidChange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifySystemKeyboardAvoidChange";
    info.bundleName_ = "NotifySystemKeyboardAvoidChange";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);

    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_CREATED);
    ASSERT_EQ(true, keyboardSession->keyboardAvoidAreaActive_);

    keyboardSession->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_CREATED);
    ASSERT_EQ(true, keyboardSession->keyboardAvoidAreaActive_);

    keyboardSession->SetIsSystemKeyboard(true);
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_CREATED);
    ASSERT_EQ(true, keyboardSession->keyboardAvoidAreaActive_);

    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCallback =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    keyboardSession->keyboardCallback_ = keyboardCallback;
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_CREATED);
    ASSERT_EQ(true, keyboardSession->keyboardAvoidAreaActive_);

    ASSERT_NE(nullptr, keyboardSession->keyboardCallback_);
    keyboardSession->keyboardCallback_->onSystemKeyboardAvoidChange = [](DisplayId displayId,
                                                                         SystemKeyboardAvoidChangeReason reason) {};
    keyboardSession->NotifySystemKeyboardAvoidChange(SystemKeyboardAvoidChangeReason::KEYBOARD_CREATED);
    ASSERT_EQ(true, keyboardSession->keyboardAvoidAreaActive_);
}

/**
 * @tc.name: ChangeKeyboardEffectOption
 * @tc.desc: test ChangeKeyboardEffectOption
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest, ChangeKeyboardEffectOption, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ChangeKeyboardEffectOption";
    info.bundleName_ = "ChangeKeyboardEffectOption";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);

    KeyboardEffectOption effectOption;
    effectOption.viewMode_ = KeyboardViewMode::DARK_IMMERSIVE_MODE;
    auto result = KeyboardViewMode::NON_IMMERSIVE_MODE;
    keyboardSession->ChangeKeyboardEffectOption(effectOption);
    ASSERT_NE(result, KeyboardViewMode::DARK_IMMERSIVE_MODE);

    keyboardSession->changeKeyboardEffectOptionFunc_ = [&result](const KeyboardEffectOption& effectOption) {
        result = effectOption.viewMode_;
    };
    keyboardSession->ChangeKeyboardEffectOption(effectOption);

    sleep(SPLIT_TEST_SLEEP_S);
    ASSERT_EQ(result, KeyboardViewMode::DARK_IMMERSIVE_MODE);
    auto lastOption = keyboardSession->property_->GetKeyboardEffectOption();
    ASSERT_EQ(lastOption.viewMode_, KeyboardViewMode::DARK_IMMERSIVE_MODE);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
