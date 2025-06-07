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
#include <regex>
#include <bundle_mgr_interface.h>
#include <bundlemgr/launcher_service.h>

#include "common_test_utils.h"
#include "context.h"
#include "interfaces/include/ws_common.h"
#include "iremote_object_mocker.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_scene_session.h"
#include "mock/mock_window_event_channel.h"
#include "session_info.h"
#include "session_manager.h"
#include "session_manager/include/scene_session_manager.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/main_session.h"
#include "window_manager_agent.h"
#include "zidl/window_manager_agent_interface.h"
#include "mock/mock_window_manager_agent_lite.h"
#include "session_manager/include/session_manager_agent_controller.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class KeyboardTestData;
class SceneSessionManagerTest12 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    // keyboard
    void ConstructKeyboardCallingWindowTestData(const KeyboardTestData& keyboardTestData);
    bool CheckKeyboardOccupiedAreaInfo(const Rect& desiredRect, const WSRect& actualRect);
    void GetKeyboardOccupiedAreaWithRotationTestData(WindowUIType windowType,
                                                     DisplayId cDisplayId,
                                                     DisplayId kDisplayId,
                                                     SessionState keyboardState,
                                                     WindowGravity gravity);

    static sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerTest12::ssm_ = nullptr;

void SceneSessionManagerTest12::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerTest12::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerTest12::SetUp() {}

void SceneSessionManagerTest12::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
}

const Rect landscapePanelRect_ = { 0, 538, 2720, 722 };
const Rect portraitPanelRect_ = { 0, 1700, 1260, 1020 };
std::vector<std::pair<bool, WSRect>> avoidAreas_ = {};
bool SceneSessionManagerTest12::CheckKeyboardOccupiedAreaInfo(const Rect& desiredRect, const WSRect& actualRect)
{
    return desiredRect.posX_ == actualRect.posX_ && desiredRect.posY_ == actualRect.posY_ &&
           static_cast<int32_t>(desiredRect.width_) == actualRect.width_ &&
           static_cast<int32_t>(desiredRect.height_) == actualRect.height_;
}

void SceneSessionManagerTest12::GetKeyboardOccupiedAreaWithRotationTestData(WindowUIType windowType,
                                                                            DisplayId cDisplayId,
                                                                            DisplayId kDisplayId,
                                                                            SessionState keyboardState,
                                                                            WindowGravity gravity)
{
    ssm_->sceneSessionMap_.clear();
    avoidAreas_.clear();
    ssm_->systemConfig_.windowUIType_ = windowType;
    SessionInfo callingSessionInfo;
    callingSessionInfo.abilityName_ = "callingSession";
    callingSessionInfo.bundleName_ = "callingSession";
    sptr<SceneSession> callingSession = sptr<SceneSession>::MakeSptr(callingSessionInfo, nullptr);
    sptr<WindowSessionProperty> callingSessionProperties = sptr<WindowSessionProperty>::MakeSptr();
    callingSessionProperties->SetDisplayId(cDisplayId);
    callingSession->SetSessionProperty(callingSessionProperties);

    SessionInfo keyboardSessionInfo;
    keyboardSessionInfo.abilityName_ = "keyboardSession";
    keyboardSessionInfo.bundleName_ = "keyboardSession";
    sptr<SceneSession> keyboardSession = sptr<SceneSession>::MakeSptr(keyboardSessionInfo, nullptr);
    keyboardSession->SetScreenId(kDisplayId);
    sptr<WindowSessionProperty> keyboardProperties = sptr<WindowSessionProperty>::MakeSptr();
    keyboardProperties->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardProperties->SetIsSystemKeyboard(false);
    KeyboardLayoutParams params;
    params.LandscapePanelRect_ = landscapePanelRect_;
    params.PortraitPanelRect_ = portraitPanelRect_;
    params.gravity_ = gravity;
    keyboardProperties->SetKeyboardLayoutParams(params);
    keyboardSession->SetSessionState(keyboardState);
    keyboardSession->SetSessionProperty(keyboardProperties);

    ssm_->sceneSessionMap_.insert({ 1, callingSession });
    ssm_->sceneSessionMap_.insert({ 2, keyboardSession });
}

class KeyboardTestData {
public:
    KeyboardTestData(uint64_t cScreenId,
                     int32_t cPid,
                     int32_t kScreenId,
                     WindowType keyboardWindowType,
                     bool isSysKeyboard)
        : cScreenId_(cScreenId),
          cPid_(cPid),
          kScreenId_(kScreenId),
          keyboardWindowType_(keyboardWindowType),
          isSysKeyboard_(isSysKeyboard)
    {
    }

    void SetCallingSessionId(uint32_t callingSessionId)
    {
        callingSessionId_ = callingSessionId;
    }

private:
    uint64_t cScreenId_; // screenId of callingWindow
    int32_t cPid_;       // callingPid of callingWindow
    int32_t kScreenId_;  // screenId of keyboard
    WindowType keyboardWindowType_;
    bool isSysKeyboard_;
    uint32_t callingSessionId_;
};

void SceneSessionManagerTest12::ConstructKeyboardCallingWindowTestData(const KeyboardTestData& keyboardTestData)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo callingSessionInfo;
    callingSessionInfo.abilityName_ = "callingSession";
    callingSessionInfo.bundleName_ = "callingSession";
    callingSessionInfo.screenId_ = keyboardTestData.cScreenId_;
    sptr<SceneSession> callingSession = sptr<SceneSession>::MakeSptr(callingSessionInfo, nullptr);
    callingSession->callingPid_ = keyboardTestData.cPid_;
    sptr<WindowSessionProperty> callingSessionProperties = sptr<WindowSessionProperty>::MakeSptr();
    callingSessionProperties->SetDisplayId(keyboardTestData.cScreenId_);
    callingSession->SetSessionProperty(callingSessionProperties);

    SessionInfo keyboardSessionInfo;
    keyboardSessionInfo.abilityName_ = "keyboardSession";
    keyboardSessionInfo.bundleName_ = "keyboardSession";
    sptr<SceneSession> keyboardSession = sptr<SceneSession>::MakeSptr(keyboardSessionInfo, nullptr);

    keyboardSession->SetScreenId(keyboardTestData.kScreenId_);
    sptr<WindowSessionProperty> keyboardProperties = sptr<WindowSessionProperty>::MakeSptr();
    keyboardProperties->SetWindowType(keyboardTestData.keyboardWindowType_);
    keyboardProperties->SetIsSystemKeyboard(keyboardTestData.isSysKeyboard_);
    keyboardProperties->SetCallingSessionId(keyboardTestData.callingSessionId_);
    keyboardSession->SetSessionProperty(keyboardProperties);

    ssm_->sceneSessionMap_.insert({ keyboardTestData.callingSessionId_, callingSession });
    ssm_->sceneSessionMap_.insert({ 2, keyboardSession });
}

namespace {
/**
 * @tc.name: GetResourceManager
 * @tc.desc: GetResourceManager
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetResourceManager, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    AppExecFwk::AbilityInfo abilityInfo;
    auto result = ssm_->GetResourceManager(abilityInfo);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: RequestKeyboardPanelSession
 * @tc.desc: test RequestKeyboardPanelSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, RequestKeyboardPanelSession, TestSize.Level1)
{
    sptr<SceneSessionManager> ssm = sptr<SceneSessionManager>::MakeSptr();
    std::string panelName = "SystemKeyboardPanel";
    ASSERT_NE(nullptr, ssm->RequestKeyboardPanelSession(panelName, 0)); // 0 is screenId
    ssm->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ASSERT_NE(nullptr, ssm->RequestKeyboardPanelSession(panelName, 0)); // 0 is screenId
}

/**
 * @tc.name: CreateKeyboardPanelSession
 * @tc.desc: CreateKeyboardPanelSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, CreateKeyboardPanelSession02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.screenId_ = SCREEN_ID_INVALID;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->CreateKeyboardPanelSession(sceneSession);
}

/**
 * @tc.name: CreateKeyboardPanelSession03
 * @tc.desc: test CreateKeyboardPanelSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, CreateKeyboardPanelSession03, TestSize.Level1)
{
    SessionInfo keyboardInfo;
    keyboardInfo.abilityName_ = "CreateKeyboardPanelSession03";
    keyboardInfo.bundleName_ = "CreateKeyboardPanelSession03";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(keyboardInfo, nullptr, nullptr);
    ASSERT_EQ(nullptr, keyboardSession->GetKeyboardPanelSession());
    sptr<SceneSessionManager> ssm = sptr<SceneSessionManager>::MakeSptr();

    // the keyboard panel enabled flag of ssm is false
    ssm->CreateKeyboardPanelSession(keyboardSession);
    ASSERT_EQ(nullptr, keyboardSession->GetKeyboardPanelSession());

    // keyboard session is nullptr
    ssm->isKeyboardPanelEnabled_ = true;
    ssm->CreateKeyboardPanelSession(nullptr);
    ASSERT_EQ(nullptr, keyboardSession->GetKeyboardPanelSession());

    // the keyboard session is system keyboard
    keyboardSession->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    ASSERT_NE(nullptr, keyboardSession->GetSessionProperty());
    keyboardSession->SetIsSystemKeyboard(true);
    ASSERT_EQ(true, keyboardSession->IsSystemKeyboard());
    ssm->CreateKeyboardPanelSession(keyboardSession);
    ASSERT_NE(nullptr, keyboardSession->GetKeyboardPanelSession());
}

/**
 * @tc.name: CreateKeyboardPanelSession04
 * @tc.desc: test CreateKeyboardPanelSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, CreateKeyboardPanelSession04, TestSize.Level1)
{
    SessionInfo keyboardInfo;
    keyboardInfo.abilityName_ = "CreateKeyboardPanelSession04";
    keyboardInfo.bundleName_ = "CreateKeyboardPanelSession04";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(keyboardInfo, nullptr, nullptr);
    ASSERT_EQ(nullptr, keyboardSession->GetKeyboardPanelSession());
    sptr<SceneSessionManager> ssm = sptr<SceneSessionManager>::MakeSptr();

    // the keyboard panel enabled flag of ssm is true
    ssm->isKeyboardPanelEnabled_ = true;
    ASSERT_NE(nullptr, keyboardSession->GetSessionProperty());
    ASSERT_EQ(false, keyboardSession->IsSystemKeyboard());
    ssm->CreateKeyboardPanelSession(keyboardSession);
    ASSERT_NE(nullptr, keyboardSession->GetKeyboardPanelSession());

    // keyboard panel session is already exists
    ssm->CreateKeyboardPanelSession(keyboardSession);
    ASSERT_NE(nullptr, keyboardSession->GetKeyboardPanelSession());
}

/**
 * @tc.name: TestCheckSystemWindowPermission_01
 * @tc.desc: Test CheckSystemWindowPermission with windowType WINDOW_TYPE_UI_EXTENSION then false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, TestCheckSystemWindowPermission_01, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    property->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    ASSERT_EQ(false, ssm_->CheckSystemWindowPermission(property));
}

/**
 * @tc.name: TestCheckSystemWindowPermission_02
 * @tc.desc: Test CheckSystemWindowPermission with windowType WINDOW_TYPE_APP_MAIN_WINDOW then true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, TestCheckSystemWindowPermission_02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW); // main window is not system window
    ASSERT_EQ(true, ssm_->CheckSystemWindowPermission(property));
}

/**
 * @tc.name: TestCheckSystemWindowPermission_03
 * @tc.desc: Test CheckSystemWindowPermission with windowType WINDOW_TYPE_INPUT_METHOD_FLOAT then true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, TestCheckSystemWindowPermission_03, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    ASSERT_EQ(true, ssm_->CheckSystemWindowPermission(property));
}

/**
 * @tc.name: TestCheckSystemWindowPermission_04
 * @tc.desc: Test CheckSystemWindowPermission with windowType WINDOW_TYPE_INPUT_METHOD_STATUS_BAR then true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, TestCheckSystemWindowPermission_04, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_STATUS_BAR);
    ASSERT_EQ(true, ssm_->CheckSystemWindowPermission(property));
}

/**
 * @tc.name: TestCheckSystemWindowPermission_05
 * @tc.desc: Test CheckSystemWindowPermission with windowType WINDOW_TYPE_INPUT_METHOD_FLOAT then false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, TestCheckSystemWindowPermission_05, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    property->SetIsSystemKeyboard(true);
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    ASSERT_EQ(false, ssm_->CheckSystemWindowPermission(property));
}

/**
 * @tc.name: TestCheckSystemWindowPermission_06
 * @tc.desc: Test CheckSystemWindowPermission with windowType WINDOW_TYPE_DIALOG then true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, TestCheckSystemWindowPermission_06, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ASSERT_EQ(true, ssm_->CheckSystemWindowPermission(property));
}

/**
 * @tc.name: TestCheckSystemWindowPermission_07
 * @tc.desc: Test CheckSystemWindowPermission with windowType WINDOW_TYPE_PIP then true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, TestCheckSystemWindowPermission_07, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    ASSERT_EQ(true, ssm_->CheckSystemWindowPermission(property));
}

/**
 * @tc.name: TestCheckSystemWindowPermission_08
 * @tc.desc: Test CheckSystemWindowPermission with windowType WINDOW_TYPE_FLOAT then true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, TestCheckSystemWindowPermission_08, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    property->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    ASSERT_EQ(false, ssm_->CheckSystemWindowPermission(property));
}

/**
 * @tc.name: TestCheckSystemWindowPermission_09
 * @tc.desc: Test CheckSystemWindowPermission with windowType WINDOW_TYPE_TOAST then true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, TestCheckSystemWindowPermission_09, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    property->SetWindowType(WindowType::WINDOW_TYPE_TOAST);
    ASSERT_EQ(true, ssm_->CheckSystemWindowPermission(property));
}

/**
 * @tc.name: TestCheckSystemWindowPermission_010
 * @tc.desc: Test CheckSystemWindowPermission with windowType WINDOW_TYPE_SYSTEM_SUB_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, TestCheckSystemWindowPermission_010, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetParentPersistentId(100101);
    property->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW);
    ASSERT_EQ(true, ssm_->CheckSystemWindowPermission(property));
}

/**
 * @tc.name: TestCheckSystemWindowPermission_011
 * @tc.desc: Test CheckSystemWindowPermission with windowType WINDOW_TYPE_SYSTEM_SUB_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, TestCheckSystemWindowPermission_011, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetParentPersistentId(100101);
    property->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW);

    sptr<WindowSessionProperty> parentProperty = sptr<WindowSessionProperty>::MakeSptr();
    parentProperty->SetPersistentId(100101);
    parentProperty->SetWindowType(WindowType::WINDOW_TYPE_TOAST);
    SessionInfo info;
    info.abilityName_ = "CheckSystemWindowPermission";
    info.bundleName_ = "CheckSystemWindowPermission";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetSessionProperty(parentProperty);

    ASSERT_EQ(true, ssm_->CheckSystemWindowPermission(property));
}

/**
 * @tc.name: TestCheckSystemWindowPermission_012
 * @tc.desc: Test CheckSystemWindowPermission with windowType WINDOW_TYPE_SYSTEM_SUB_WINDOW
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, TestCheckSystemWindowPermission_012, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetParentPersistentId(100101);
    property->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_SUB_WINDOW);

    sptr<WindowSessionProperty> parentProperty = sptr<WindowSessionProperty>::MakeSptr();
    parentProperty->SetPersistentId(100101);
    parentProperty->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    SessionInfo info;
    info.abilityName_ = "CheckSystemWindowPermission";
    info.bundleName_ = "CheckSystemWindowPermission";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetSessionProperty(parentProperty);

    ASSERT_EQ(true, ssm_->CheckSystemWindowPermission(property));
}

/**
 * @tc.name: CreateAndConnectSpecificSession
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, CreateAndConnectSpecificSession03, TestSize.Level1)
{
    sptr<ISessionStage> sessionStage;
    sptr<IWindowEventChannel> eventChannel;
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sptr<ISession> session;
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token;
    int32_t id = 0;
    ASSERT_NE(ssm_, nullptr);

    property->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    auto res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(WSError::WS_ERROR_NOT_SYSTEM_APP, res);

    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    property->SetTopmost(true);
    uint32_t flags = property->GetWindowFlags() & (~(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_MODAL)));
    property->SetWindowFlags(flags);
    res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(WSError::WS_ERROR_NOT_SYSTEM_APP, res);

    property->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    property->SetFloatingWindowAppType(true);
    ssm_->shouldHideNonSecureFloatingWindows_.store(true);
    res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(WSError::WS_ERROR_NOT_SYSTEM_APP, res);

    property->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW);
    res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_WINDOW, res);

    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    ssm_->isScreenLocked_ = true;
    res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(WSError::WS_DO_NOTHING, res);
}

/**
 * @tc.name: SetCreateKeyboardSessionListener
 * @tc.desc: SetCreateKeyboardSessionListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, SetCreateKeyboardSessionListener, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->SetCreateSystemSessionListener(nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "test1";
    sessionInfo.abilityName_ = "test2";
    sessionInfo.abilityInfo = nullptr;
    sessionInfo.isAtomicService_ = true;
    sessionInfo.screenId_ = SCREEN_ID_INVALID;
    ssm_->NotifySessionTouchOutside(123);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSessionInner
 * @tc.desc: check func DestroyAndDisconnectSpecificSessionInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, DestroyAndDisconnectSpecificSessionInner02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    std::vector<int32_t> recoveredPersistentIds = { 0, 1, 2 };
    ssm_->SetAlivePersistentIds(recoveredPersistentIds);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    auto ret = ssm_->DestroyAndDisconnectSpecificSessionInner(1);
    EXPECT_EQ(ret, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: DestroyToastSession
 * @tc.desc: DestroyToastSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, DestroyToastSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.screenId_ = SCREEN_ID_INVALID;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->DestroyToastSession(sceneSession);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->DestroyToastSession(sceneSession);
    ssm_->StartUIAbilityBySCB(sceneSession);
    int32_t ret = ssm_->ChangeUIAbilityVisibilityBySCB(sceneSession, true, false);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: DestroyToastSession
 * @tc.desc: DestroyToastSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, DestroyToastSession02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.screenId_ = SCREEN_ID_INVALID;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->DestroyToastSession(sceneSession);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    ssm_->DestroyToastSession(sceneSession);
}

/**
 * @tc.name: CheckModalSubWindowPermission
 * @tc.desc: CheckModalSubWindowPermission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, CheckModalSubWindowPermission, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property->SetWindowFlags(123);
    ssm_->CheckModalSubWindowPermission(property);
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ssm_->CheckModalSubWindowPermission(property);
}

/**
 * @tc.name: CheckModalSubWindowPermission
 * @tc.desc: CheckModalSubWindowPermission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, CheckModalSubWindowPermission02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property->SetWindowFlags(123);
    property->SetTopmost(true);
    ssm_->CheckModalSubWindowPermission(property);
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ssm_->CheckModalSubWindowPermission(property);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSessionInner
 * @tc.desc: check func DestroyAndDisconnectSpecificSessionInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, DestroyAndDisconnectSpecificSessionInner, TestSize.Level1)
{
    sptr<ISession> session;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    std::vector<int32_t> recoveredPersistentIds = { 0, 1, 2 };
    ssm_->SetAlivePersistentIds(recoveredPersistentIds);
    ProcessShiftFocusFunc shiftFocusFunc_;
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    auto ret = ssm_->DestroyAndDisconnectSpecificSessionInner(1);
    EXPECT_EQ(ret, WSError::WS_ERROR_NULLPTR);
    property->SetPersistentId(1);
    ret = ssm_->DestroyAndDisconnectSpecificSessionInner(1);
    EXPECT_EQ(ret, WSError::WS_ERROR_NULLPTR);

    property->SetWindowType(WindowType::WINDOW_TYPE_TOAST);
    ret = ssm_->DestroyAndDisconnectSpecificSessionInner(1);
    EXPECT_EQ(ret, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSessionWithDetachCallback
 * @tc.desc: SceneSesionManager destroy and disconnect specific session with detach callback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, DestroyAndDetachCallback, TestSize.Level1)
{
    int32_t persistentId = 0;
    ASSERT_NE(ssm_, nullptr);
    sptr<IRemoteObject> callback = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(callback, nullptr);
    ssm_->DestroyAndDisconnectSpecificSessionWithDetachCallback(persistentId, callback);
    sptr<WindowSessionProperty> property;
    ssm_->recoveringFinished_ = false;
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, property);
    ssm_->DestroyAndDisconnectSpecificSessionWithDetachCallback(persistentId, callback);
}

/**
 * @tc.name: IsKeyboardForeground
 * @tc.desc: IsKeyboardForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, IsKeyboardForeground, TestSize.Level1)
{
    auto sceneSessionManager = sptr<SceneSessionManager>::MakeSptr();
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->property_ = sptr<WindowSessionProperty>::MakeSptr();
    sceneSessionManager->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    ASSERT_EQ(true, sceneSessionManager->IsKeyboardForeground());
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW);
    ASSERT_EQ(false, sceneSessionManager->IsKeyboardForeground());
    sceneSessionManager->sceneSessionMap_.clear();
    ASSERT_EQ(false, sceneSessionManager->IsKeyboardForeground());
}

/**
 * @tc.name: RegisterWatchGestureConsumeResultCallback
 * @tc.desc: RegisterWatchGestureConsumeResultCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, RegisterWatchGestureConsumeResultCallback, TestSize.Level1)
{
    NotifyWatchGestureConsumeResultFunc func = [](int32_t keyCode, bool isConsumed) {};
    ssm_->RegisterWatchGestureConsumeResultCallback(std::move(func));
    ASSERT_NE(ssm_->onWatchGestureConsumeResultFunc_, nullptr);
}

/**
 * @tc.name: RegisterWatchFocusActiveChangeCallback
 * @tc.desc: RegisterWatchFocusActiveChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, RegisterWatchFocusActiveChangeCallback, TestSize.Level1)
{
    NotifyWatchFocusActiveChangeFunc func = [](bool isActive) {};
    ssm_->RegisterWatchFocusActiveChangeCallback(std::move(func));
    ASSERT_NE(ssm_->onWatchFocusActiveChangeFunc_, nullptr);
}

/**
 * @tc.name: NotifyWatchGestureConsumeResult
 * @tc.desc: NotifyWatchGestureConsumeResult
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, NotifyWatchGestureConsumeResult, TestSize.Level1)
{
    int32_t keyCode = 2049;
    bool isConsumed = true;
    ssm_->onWatchGestureConsumeResultFunc_ = [](int32_t keyCode, bool isConsumed) {};
    auto ret = ssm_->NotifyWatchGestureConsumeResult(keyCode, isConsumed);
    ASSERT_EQ(WMError::WM_OK, ret);
    ssm_->onWatchGestureConsumeResultFunc_ = nullptr;
    ret = ssm_->NotifyWatchGestureConsumeResult(keyCode, isConsumed);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: NotifyWatchFocusActiveChange
 * @tc.desc: NotifyWatchFocusActiveChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, NotifyWatchFocusActiveChange, TestSize.Level1)
{
    bool isActive = true;
    ssm_->onWatchFocusActiveChangeFunc_ = [](bool isActive) {};
    auto ret = ssm_->NotifyWatchFocusActiveChange(isActive);
    ASSERT_EQ(WMError::WM_OK, ret);
    ssm_->onWatchFocusActiveChangeFunc_ = nullptr;
    ret = ssm_->NotifyWatchFocusActiveChange(isActive);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: ShiftAppWindowPointerEvent01
 * @tc.desc: ShiftAppWindowPointerEvent,
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, ShiftAppWindowPointerEvent_01, TestSize.Level1)
{
    SessionInfo sourceInfo;
    sourceInfo.windowType_ = 1;
    sptr<SceneSession> sourceSceneSession = sptr<SceneSession>::MakeSptr(sourceInfo, nullptr);
    ssm_->sceneSessionMap_.insert({ sourceSceneSession->GetPersistentId(), sourceSceneSession });

    SessionInfo targetInfo;
    targetInfo.windowType_ = 1;
    sptr<SceneSession> targetSceneSession = sptr<SceneSession>::MakeSptr(targetInfo, nullptr);
    ssm_->sceneSessionMap_.insert({ targetSceneSession->GetPersistentId(), targetSceneSession });

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    int32_t fingerId = 0;
    WMError result = ssm_->ShiftAppWindowPointerEvent(INVALID_SESSION_ID, targetSceneSession->GetPersistentId(),
        fingerId);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_SESSION);
    result = ssm_->ShiftAppWindowPointerEvent(sourceSceneSession->GetPersistentId(), INVALID_SESSION_ID, fingerId);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_SESSION);
    ssm_->sceneSessionMap_.erase(sourceSceneSession->GetPersistentId());
    ssm_->sceneSessionMap_.erase(targetSceneSession->GetPersistentId());
}

/**
 * @tc.name: ShiftAppWindowPointerEvent02
 * @tc.desc: ShiftAppWindowPointerEvent,
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, ShiftAppWindowPointerEvent_02, TestSize.Level1)
{
    SessionInfo systemWindowInfo;
    systemWindowInfo.windowType_ = 2000;
    sptr<SceneSession> systemWindowSession = sptr<SceneSession>::MakeSptr(systemWindowInfo, nullptr);
    ssm_->sceneSessionMap_.insert({ systemWindowSession->GetPersistentId(), systemWindowSession });

    SessionInfo mainWindowInfo;
    mainWindowInfo.windowType_ = 1;
    sptr<SceneSession> mainWindowSession = sptr<SceneSession>::MakeSptr(mainWindowInfo, nullptr);
    ssm_->sceneSessionMap_.insert({ mainWindowSession->GetPersistentId(), mainWindowSession });

    int mainWindowPersistentId = mainWindowSession->GetPersistentId();
    int systemWindowPersistentId = systemWindowSession->GetPersistentId();
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    int32_t fingerId = 0;
    WMError result = ssm_->ShiftAppWindowPointerEvent(mainWindowPersistentId, systemWindowPersistentId, fingerId);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_CALLING);
    result = ssm_->ShiftAppWindowPointerEvent(systemWindowPersistentId, mainWindowPersistentId, fingerId);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_CALLING);
    ssm_->sceneSessionMap_.erase(systemWindowSession->GetPersistentId());
    ssm_->sceneSessionMap_.erase(mainWindowSession->GetPersistentId());
}

/**
 * @tc.name: ShiftAppWindowPointerEvent03
 * @tc.desc: ShiftAppWindowPointerEvent,
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, ShiftAppWindowPointerEvent_03, TestSize.Level1)
{
    SessionInfo sourceInfo;
    sourceInfo.windowType_ = 1;
    sptr<SceneSession> sourceSceneSession = sptr<SceneSession>::MakeSptr(sourceInfo, nullptr);
    ssm_->sceneSessionMap_.insert({ sourceSceneSession->GetPersistentId(), sourceSceneSession });

    int32_t sourcePersistentId = sourceSceneSession->GetPersistentId();
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    int32_t fingerId = 0;
    WMError result = ssm_->ShiftAppWindowPointerEvent(sourcePersistentId, sourcePersistentId, fingerId);
    EXPECT_EQ(result, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    result = ssm_->ShiftAppWindowPointerEvent(sourcePersistentId, sourcePersistentId, fingerId);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_CALLING);
    ssm_->sceneSessionMap_.erase(sourceSceneSession->GetPersistentId());
}

/**
 * @tc.name: ShiftAppWindowPointerEvent04
 * @tc.desc: ShiftAppWindowPointerEvent,
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, ShiftAppWindowPointerEvent_04, TestSize.Level1)
{
    SessionInfo sourceInfo;
    sourceInfo.windowType_ = 1;
    sptr<SceneSession> sourceSceneSession = sptr<SceneSession>::MakeSptr(sourceInfo, nullptr);
    ssm_->sceneSessionMap_.insert({ sourceSceneSession->GetPersistentId(), sourceSceneSession });

    SessionInfo otherSourceInfo;
    otherSourceInfo.bundleName_ = "other";
    otherSourceInfo.windowType_ = 1;
    sptr<SceneSession> otherSourceSession = sptr<SceneSession>::MakeSptr(otherSourceInfo, nullptr);
    ssm_->sceneSessionMap_.insert({ otherSourceSession->GetPersistentId(), otherSourceSession });

    SessionInfo otherTargetInfo;
    otherTargetInfo.bundleName_ = "other";
    otherTargetInfo.windowType_ = 1;
    sptr<SceneSession> otherTargetSession = sptr<SceneSession>::MakeSptr(otherTargetInfo, nullptr);
    ssm_->sceneSessionMap_.insert({ otherTargetSession->GetPersistentId(), otherTargetSession });

    int32_t sourcePersistentId = sourceSceneSession->GetPersistentId();
    int32_t otherSourcePersistentId = otherSourceSession->GetPersistentId();
    int32_t otherTargetPersistentId = otherTargetSession->GetPersistentId();
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    int32_t fingerId = 0;
    WMError result = ssm_->ShiftAppWindowPointerEvent(sourcePersistentId, otherTargetPersistentId, fingerId);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_CALLING);
    result = ssm_->ShiftAppWindowPointerEvent(otherSourcePersistentId, otherTargetPersistentId, fingerId);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_CALLING);
    ssm_->sceneSessionMap_.erase(sourceSceneSession->GetPersistentId());
    ssm_->sceneSessionMap_.erase(otherSourceSession->GetPersistentId());
    ssm_->sceneSessionMap_.erase(otherTargetSession->GetPersistentId());
}

/**
 * @tc.name: ShiftAppWindowPointerEventInner01
 * @tc.desc: ShiftAppWindowPointerEventInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, ShiftAppWindowPointerEventInner01, TestSize.Level1)
{
    SessionInfo sourceInfo;
    sourceInfo.windowType_ = 1;
    sptr<SceneSession> sourceSceneSession = sptr<SceneSession>::MakeSptr(sourceInfo, nullptr);
    ssm_->sceneSessionMap_.insert({ sourceSceneSession->GetPersistentId(), sourceSceneSession });

    SessionInfo targetInfo;
    targetInfo.windowType_ = 1;
    sptr<SceneSession> targetSceneSession = sptr<SceneSession>::MakeSptr(targetInfo, nullptr);
    ssm_->sceneSessionMap_.insert({ targetSceneSession->GetPersistentId(), targetSceneSession });

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    int32_t fingerId = 0;
    WMError result = ssm_->ShiftAppWindowPointerEventInner(
        sourceSceneSession->GetPersistentId(), targetSceneSession->GetPersistentId(), DISPLAY_ID_INVALID, fingerId);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_CALLING);
    ssm_->sceneSessionMap_.erase(sourceSceneSession->GetPersistentId());
    ssm_->sceneSessionMap_.erase(targetSceneSession->GetPersistentId());
}

/**
 * @tc.name: ShiftAppWindowPointerEventInner02
 * @tc.desc: ShiftAppWindowPointerEventInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, ShiftAppWindowPointerEventInner02, TestSize.Level1)
{
    SessionInfo sourceInfo;
    sourceInfo.windowType_ = 1;
    sptr<SceneSession> sourceSceneSession = sptr<SceneSession>::MakeSptr(sourceInfo, nullptr);
    ssm_->sceneSessionMap_.insert({ sourceSceneSession->GetPersistentId(), sourceSceneSession });

    SessionInfo targetInfo;
    targetInfo.windowType_ = 1;
    sptr<SceneSession> targetSceneSession = sptr<SceneSession>::MakeSptr(targetInfo, nullptr);
    ssm_->sceneSessionMap_.insert({ targetSceneSession->GetPersistentId(), targetSceneSession });

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    int32_t fingerId = 0;
    WMError result = ssm_->ShiftAppWindowPointerEventInner(
        sourceSceneSession->GetPersistentId(), targetSceneSession->GetPersistentId(), 0, fingerId);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_CALLING);
    ssm_->sceneSessionMap_.erase(sourceSceneSession->GetPersistentId());
    ssm_->sceneSessionMap_.erase(targetSceneSession->GetPersistentId());
}

/**
 * @tc.name: GetKeyboardSession
 * @tc.desc: test GetKeyboardSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetKeyboardSession, TestSize.Level1)
{
    sptr<SceneSessionManager> ssm = sptr<SceneSessionManager>::MakeSptr();
    SessionInfo info;
    info.abilityName_ = "GetKeyboardSession";
    info.bundleName_ = "GetKeyboardSession";
    info.windowType_ = 2105; // 2105 is WINDOW_TYPE_INPUT_METHOD_FLOAT
    info.screenId_ = 1;      // 1 is screenId
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_EQ(false, keyboardSession->IsSystemKeyboard());
    ssm->sceneSessionMap_.insert({ keyboardSession->GetPersistentId(), keyboardSession });
    sptr<KeyboardSession> systemKeyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    systemKeyboardSession->SetIsSystemKeyboard(true);
    ASSERT_EQ(true, systemKeyboardSession->IsSystemKeyboard());
    ssm->sceneSessionMap_.insert({ systemKeyboardSession->GetPersistentId(), systemKeyboardSession });

    ASSERT_EQ(nullptr, ssm->GetKeyboardSession(DISPLAY_ID_INVALID, false));
    ASSERT_NE(nullptr, ssm->GetKeyboardSession(keyboardSession->GetScreenId(), false));
    ASSERT_NE(nullptr, ssm->GetKeyboardSession(systemKeyboardSession->GetScreenId(), true));
}

/**
 * @tc.name: UpdateKeyboardAvoidAreaActive
 * @tc.desc: test UpdateKeyboardAvoidAreaActive
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, UpdateKeyboardAvoidAreaActive, TestSize.Level1)
{
    sptr<SceneSessionManager> ssm = sptr<SceneSessionManager>::MakeSptr();
    SessionInfo info;
    info.abilityName_ = "UpdateKeyboardAvoidAreaActive";
    info.bundleName_ = "UpdateKeyboardAvoidAreaActive";
    info.windowType_ = 2105; // 2105 is WINDOW_TYPE_INPUT_METHOD_FLOAT
    info.screenId_ = 1;      // 1 is screenId
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(nullptr, keyboardSession->GetSessionProperty());
    keyboardSession->GetSessionProperty()->SetDisplayId(info.screenId_);
    ASSERT_EQ(false, keyboardSession->IsSystemKeyboard());
    ssm->sceneSessionMap_.insert({ keyboardSession->GetPersistentId(), keyboardSession });
    sptr<KeyboardSession> systemKeyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(nullptr, systemKeyboardSession->GetSessionProperty());
    systemKeyboardSession->GetSessionProperty()->SetDisplayId(info.screenId_);
    systemKeyboardSession->SetIsSystemKeyboard(true);
    ASSERT_EQ(true, systemKeyboardSession->IsSystemKeyboard());
    ssm->sceneSessionMap_.insert({ systemKeyboardSession->GetPersistentId(), systemKeyboardSession });

    ssm->UpdateKeyboardAvoidAreaActive(false);
    ASSERT_EQ(true, keyboardSession->keyboardAvoidAreaActive_);
    ASSERT_EQ(false, systemKeyboardSession->keyboardAvoidAreaActive_);
    ssm->UpdateKeyboardAvoidAreaActive(true);
    ASSERT_EQ(false, keyboardSession->keyboardAvoidAreaActive_);
    ASSERT_EQ(true, systemKeyboardSession->keyboardAvoidAreaActive_);
}

/**
 * @tc.name: HandleKeyboardAvoidChange
 * @tc.desc: test HandleKeyboardAvoidChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, HandleKeyboardAvoidChange, TestSize.Level1)
{
    sptr<SceneSessionManager> ssm = sptr<SceneSessionManager>::MakeSptr();
    SessionInfo info;
    info.abilityName_ = "HandleKeyboardAvoidChange";
    info.bundleName_ = "HandleKeyboardAvoidChange";
    info.windowType_ = 2105; // 2105 is WINDOW_TYPE_INPUT_METHOD_FLOAT
    info.screenId_ = 1;      // 1 is screenId
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(nullptr, keyboardSession->GetSessionProperty());
    keyboardSession->GetSessionProperty()->SetDisplayId(info.screenId_);
    ASSERT_EQ(false, keyboardSession->IsSystemKeyboard());
    ssm->sceneSessionMap_.insert({ keyboardSession->GetPersistentId(), keyboardSession });
    sptr<KeyboardSession> systemKeyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    ASSERT_NE(nullptr, systemKeyboardSession->GetSessionProperty());
    systemKeyboardSession->GetSessionProperty()->SetDisplayId(info.screenId_);
    systemKeyboardSession->SetIsSystemKeyboard(true);
    ASSERT_EQ(true, systemKeyboardSession->IsSystemKeyboard());
    ssm->sceneSessionMap_.insert({ systemKeyboardSession->GetPersistentId(), systemKeyboardSession });

    ssm->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ssm->HandleKeyboardAvoidChange(
        keyboardSession, keyboardSession->GetScreenId(), SystemKeyboardAvoidChangeReason::KEYBOARD_CREATED);
    ASSERT_EQ(true, keyboardSession->keyboardAvoidAreaActive_);

    ssm->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ssm->HandleKeyboardAvoidChange(
        keyboardSession, keyboardSession->GetScreenId(), SystemKeyboardAvoidChangeReason::KEYBOARD_CREATED);
    ASSERT_EQ(true, keyboardSession->keyboardAvoidAreaActive_);

    ssm->HandleKeyboardAvoidChange(
        systemKeyboardSession, systemKeyboardSession->GetScreenId(), SystemKeyboardAvoidChangeReason::KEYBOARD_SHOW);
    ASSERT_EQ(false, keyboardSession->keyboardAvoidAreaActive_);
    ASSERT_EQ(true, systemKeyboardSession->keyboardAvoidAreaActive_);

    ssm->HandleKeyboardAvoidChange(systemKeyboardSession,
                                   systemKeyboardSession->GetScreenId(),
                                   SystemKeyboardAvoidChangeReason::KEYBOARD_GRAVITY_BOTTOM);
    ASSERT_EQ(false, keyboardSession->keyboardAvoidAreaActive_);
    ASSERT_EQ(true, systemKeyboardSession->keyboardAvoidAreaActive_);

    ssm->HandleKeyboardAvoidChange(
        systemKeyboardSession, systemKeyboardSession->GetScreenId(), SystemKeyboardAvoidChangeReason::KEYBOARD_HIDE);
    ASSERT_EQ(true, keyboardSession->keyboardAvoidAreaActive_);
    ASSERT_EQ(false, systemKeyboardSession->keyboardAvoidAreaActive_);

    ssm->HandleKeyboardAvoidChange(systemKeyboardSession,
                                   systemKeyboardSession->GetScreenId(),
                                   SystemKeyboardAvoidChangeReason::KEYBOARD_DISCONNECT);
    ASSERT_EQ(true, keyboardSession->keyboardAvoidAreaActive_);
    ASSERT_EQ(false, systemKeyboardSession->keyboardAvoidAreaActive_);

    ssm->HandleKeyboardAvoidChange(systemKeyboardSession,
                                   systemKeyboardSession->GetScreenId(),
                                   SystemKeyboardAvoidChangeReason::KEYBOARD_GRAVITY_FLOAT);
    ASSERT_EQ(true, keyboardSession->keyboardAvoidAreaActive_);
    ASSERT_EQ(false, systemKeyboardSession->keyboardAvoidAreaActive_);
}

/**
 * @tc.name: GetAllWindowLayoutInfo01
 * @tc.desc: HALF_FOLDED
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetAllWindowLayoutInfo01, TestSize.Level0)
{
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(
        0, SuperFoldStatus::HALF_FOLDED, { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession1->SetVisibilityState(WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    WSRect rect = { 0, 1500, 120, 1000 };
    sceneSession1->SetSessionRect(rect);
    sceneSession1->SetSessionGlobalRect(rect);
    int32_t zOrder = 100;
    sceneSession1->SetZOrder(zOrder);
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });

    constexpr DisplayId VIRTUAL_DISPLAY_ID = 999;
    std::vector<sptr<WindowLayoutInfo>> info;
    ssm_->GetAllWindowLayoutInfo(VIRTUAL_DISPLAY_ID, info);
    ssm_->sceneSessionMap_.clear();
    ASSERT_NE(info.size(), 0);
    ASSERT_EQ(-972, info[0]->rect.posY_);
}

/**
 * @tc.name: FilterForGetAllWindowLayoutInfo01
 * @tc.desc: test return by zOrder
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, FilterForGetAllWindowLayoutInfo01, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession1->SetVisibilityState(WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    WSRect rect = { 0, 0, 120, 120 };
    sceneSession1->SetSessionRect(rect);
    sceneSession1->SetSessionGlobalRect(rect);
    int32_t zOrder = 100;
    sceneSession1->SetZOrder(zOrder);
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession2->SetVisibilityState(WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    rect = { 0, 130, 120, 120 };
    sceneSession2->SetSessionRect(rect);
    sceneSession2->SetSessionGlobalRect(rect);
    zOrder = 101;
    sceneSession2->SetZOrder(zOrder);
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });

    constexpr DisplayId DEFAULT_DISPLAY_ID = 0;
    std::vector<sptr<SceneSession>> filteredSessions;
    ssm_->FilterForGetAllWindowLayoutInfo(DEFAULT_DISPLAY_ID, false, filteredSessions);
    ssm_->sceneSessionMap_.clear();
    ASSERT_NE(filteredSessions.size(), 0);
    ASSERT_EQ(130, filteredSessions[0]->GetSessionRect().posY_);
}

/**
 * @tc.name: FilterForGetAllWindowLayoutInfo02
 * @tc.desc: test system window
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, FilterForGetAllWindowLayoutInfo02, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession1->SetVisibilityState(WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    WSRect rect = { 0, 0, 120, 120 };
    sceneSession1->SetSessionRect(rect);
    sceneSession1->SetSessionGlobalRect(rect);
    int32_t zOrder = 100;
    sceneSession1->SetZOrder(zOrder);
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });

    sessionInfo.isSystem_ = true;
    sessionInfo.abilityName_ = "SCBSmartDock";
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession2->SetVisibilityState(WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    rect = { 0, 130, 120, 120 };
    sceneSession2->SetSessionRect(rect);
    sceneSession2->SetSessionGlobalRect(rect);
    zOrder = 101;
    sceneSession2->SetZOrder(zOrder);
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });

    sessionInfo.abilityName_ = "TestAbility";
    sptr<SceneSession> sceneSession3 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession3->SetVisibilityState(WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    rect = { 0, 250, 120, 120 };
    sceneSession3->SetSessionRect(rect);
    sceneSession3->SetSessionGlobalRect(rect);
    zOrder = 102;
    sceneSession3->SetZOrder(zOrder);
    ssm_->sceneSessionMap_.insert({ sceneSession3->GetPersistentId(), sceneSession3 });

    constexpr DisplayId DEFAULT_DISPLAY_ID = 0;
    std::vector<sptr<SceneSession>> filteredSessions;
    ssm_->FilterForGetAllWindowLayoutInfo(DEFAULT_DISPLAY_ID, false, filteredSessions);
    ssm_->sceneSessionMap_.clear();
    ASSERT_EQ(2, filteredSessions.size());
}

/**
 * @tc.name: FilterForGetAllWindowLayoutInfo03
 * @tc.desc: test VisibilityState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, FilterForGetAllWindowLayoutInfo03, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession1->SetVisibilityState(WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    WSRect rect = { 0, 0, 120, 120 };
    sceneSession1->SetSessionRect(rect);
    sceneSession1->SetSessionGlobalRect(rect);
    int32_t zOrder = 101;
    sceneSession1->SetZOrder(zOrder);
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession2->SetVisibilityState(WINDOW_VISIBILITY_STATE_PARTICALLY_OCCLUSION);
    rect = { 0, 0, 130, 120 };
    sceneSession2->SetSessionRect(rect);
    sceneSession2->SetSessionGlobalRect(rect);
    zOrder = 100;
    sceneSession2->SetZOrder(zOrder);
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });

    sptr<SceneSession> sceneSession3 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession3->SetVisibilityState(WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
    rect = { 0, 0, 100, 100 };
    sceneSession3->SetSessionRect(rect);
    sceneSession3->SetSessionGlobalRect(rect);
    zOrder = 99;
    sceneSession3->SetZOrder(zOrder);
    ssm_->sceneSessionMap_.insert({ sceneSession3->GetPersistentId(), sceneSession3 });

    constexpr DisplayId DEFAULT_DISPLAY_ID = 0;
    std::vector<sptr<SceneSession>> filteredSessions;
    ssm_->FilterForGetAllWindowLayoutInfo(DEFAULT_DISPLAY_ID, false, filteredSessions);
    ssm_->sceneSessionMap_.clear();
    ASSERT_EQ(2, filteredSessions.size());
}

/**
 * @tc.name: FilterForGetAllWindowLayoutInfo04
 * @tc.desc: HALF_FOLDED
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, FilterForGetAllWindowLayoutInfo04, TestSize.Level0)
{
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(
        0, SuperFoldStatus::HALF_FOLDED, { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession1->SetVisibilityState(WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    WSRect rect = { 0, 0, 120, 120 };
    sceneSession1->SetSessionRect(rect);
    sceneSession1->SetSessionGlobalRect(rect);
    int32_t zOrder = 100;
    sceneSession1->SetZOrder(zOrder);
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession2->SetVisibilityState(WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    rect = { 0, 4000, 120, 120 };
    sceneSession2->SetSessionRect(rect);
    sceneSession2->SetSessionGlobalRect(rect);
    zOrder = 101;
    sceneSession2->SetZOrder(zOrder);
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });

    sptr<SceneSession> sceneSession3 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession3->SetVisibilityState(WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    rect = { 0, 1500, 120, 1000 };
    sceneSession3->SetSessionRect(rect);
    sceneSession3->SetSessionGlobalRect(rect);
    zOrder = 102;
    sceneSession3->SetZOrder(zOrder);
    ssm_->sceneSessionMap_.insert({ sceneSession3->GetPersistentId(), sceneSession3 });

    constexpr DisplayId DEFAULT_DISPLAY_ID = 0;
    std::vector<sptr<SceneSession>> filteredSessions1;
    ssm_->FilterForGetAllWindowLayoutInfo(DEFAULT_DISPLAY_ID, false, filteredSessions1);
    EXPECT_EQ(2, filteredSessions1.size());
    std::vector<sptr<SceneSession>> filteredSessions2;
    ssm_->FilterForGetAllWindowLayoutInfo(DEFAULT_DISPLAY_ID, true, filteredSessions2);
    ssm_->sceneSessionMap_.clear();
    ASSERT_EQ(2, filteredSessions2.size());
}

/**
 * @tc.name: FilterForGetAllWindowLayoutInfo05
 * @tc.desc: session is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, FilterForGetAllWindowLayoutInfo05, TestSize.Level0)
{
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    constexpr DisplayId DEFAULT_DISPLAY_ID = 0;
    std::vector<sptr<SceneSession>> filteredSessions;
    ssm_->FilterForGetAllWindowLayoutInfo(DEFAULT_DISPLAY_ID, false, filteredSessions);
    ssm_->sceneSessionMap_.clear();
    ASSERT_EQ(0, filteredSessions.size());
}

/**
 * @tc.name: GetFoldLowerScreenPosY01
 * @tc.desc: test get fold lower screen posY
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetFoldLowerScreenPosY01, TestSize.Level0)
{
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(
        0, SuperFoldStatus::HALF_FOLDED, { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    ASSERT_EQ(2472, ssm_->GetFoldLowerScreenPosY());
}

/**
 * @tc.name: IsGetWindowLayoutInfoNeeded01
 * @tc.desc: not System
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, IsGetWindowLayoutInfoNeeded01, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_EQ(true, ssm_->IsGetWindowLayoutInfoNeeded(sceneSession));
}

/**
 * @tc.name: IsGetWindowLayoutInfoNeeded02
 * @tc.desc: is System, not in whitelist
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, IsGetWindowLayoutInfoNeeded02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = true;
    sessionInfo.abilityName_ = "TestAbility";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_EQ(false, ssm_->IsGetWindowLayoutInfoNeeded(sceneSession));
}

/**
 * @tc.name: IsGetWindowLayoutInfoNeeded03
 * @tc.desc: is System, in whitelist
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, IsGetWindowLayoutInfoNeeded03, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = true;
    sessionInfo.abilityName_ = "SCBSmartDock";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_EQ(true, ssm_->IsGetWindowLayoutInfoNeeded(sceneSession));
}

/**
 * @tc.name: GetGlobalWindowMode01
 * @tc.desc: test window of type fullscreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetGlobalWindowMode01, TestSize.Level0)
{
    SessionInfo sessionInfo1;
    sessionInfo1.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    sceneSession1->SetRSVisible(true);
    sceneSession1->SetSessionState(SessionState::STATE_FOREGROUND);
    WSRect rect1 = { 0, 100, 1200, 1000 };
    sceneSession1->SetSessionRect(rect1);
    sceneSession1->SetSessionGlobalRect(rect1);
    sceneSession1->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });

    SessionInfo sessionInfo2;
    sessionInfo2.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    sceneSession2->SetRSVisible(true);
    sceneSession2->SetSessionState(SessionState::STATE_BACKGROUND);
    WSRect rect2 = { 0, 2100, 200, 100 };
    sceneSession2->SetSessionRect(rect2);
    sceneSession2->SetSessionGlobalRect(rect2);
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });

    SessionInfo sessionInfo3;
    sessionInfo3.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession3 = sptr<SceneSession>::MakeSptr(sessionInfo3, nullptr);
    sceneSession3->GetSessionProperty()->SetDisplayId(100);
    ssm_->sceneSessionMap_.insert({ sceneSession3->GetPersistentId(), sceneSession3 });

    SessionInfo sessionInfo4;
    sessionInfo4.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession4 = sptr<SceneSession>::MakeSptr(sessionInfo4, nullptr);
    sceneSession4->SetSessionState(SessionState::STATE_BACKGROUND);
    sceneSession4->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    ssm_->sceneSessionMap_.insert({ sceneSession4->GetPersistentId(), sceneSession4 });

    GlobalWindowMode globalWinMode = GlobalWindowMode::UNKNOWN;
    ssm_->sceneSessionMap_.insert({ -1, nullptr });
    ssm_->GetGlobalWindowMode(DEFAULT_DISPLAY_ID, globalWinMode);
    EXPECT_EQ(static_cast<uint32_t>(globalWinMode), 1);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: GetGlobalWindowMode02
 * @tc.desc: test all window modes except for fullscreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetGlobalWindowMode02, TestSize.Level0)
{
    SessionInfo sessionInfo1;
    sessionInfo1.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    sceneSession1->SetRSVisible(true);
    sceneSession1->SetSessionState(SessionState::STATE_FOREGROUND);
    WSRect rect1 = { 0, 0, 100, 100 };
    sceneSession1->SetSessionRect(rect1);
    sceneSession1->SetSessionGlobalRect(rect1);
    sceneSession1->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_PRIMARY);
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });
    GlobalWindowMode globalWinMode1 = GlobalWindowMode::UNKNOWN;
    ssm_->GetGlobalWindowMode(DEFAULT_DISPLAY_ID, globalWinMode1);
    EXPECT_EQ(static_cast<uint32_t>(globalWinMode1), 2);

    SessionInfo sessionInfo2;
    sessionInfo2.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    sceneSession2->SetRSVisible(true);
    sceneSession2->SetSessionState(SessionState::STATE_FOREGROUND);
    WSRect rect2 = { 100, 0, 100, 100 };
    sceneSession2->SetSessionRect(rect2);
    sceneSession2->SetSessionGlobalRect(rect2);
    sceneSession2->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });
    GlobalWindowMode globalWinMode2 = GlobalWindowMode::UNKNOWN;
    ssm_->GetGlobalWindowMode(DEFAULT_DISPLAY_ID, globalWinMode2);
    EXPECT_EQ(static_cast<uint32_t>(globalWinMode2), 6);

    SessionInfo sessionInfo3;
    sessionInfo3.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_PIP);
    sptr<SceneSession> sceneSession3 = sptr<SceneSession>::MakeSptr(sessionInfo3, nullptr);
    sceneSession3->SetRSVisible(true);
    sceneSession3->SetSessionState(SessionState::STATE_FOREGROUND);
    WSRect rect3 = { 0, 200, 1200, 1000 };
    sceneSession3->SetSessionRect(rect3);
    sceneSession3->SetSessionGlobalRect(rect3);
    sceneSession3->GetSessionProperty()->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    ssm_->sceneSessionMap_.insert({ sceneSession3->GetPersistentId(), sceneSession3 });
    GlobalWindowMode globalWinMode3 = GlobalWindowMode::UNKNOWN;
    ssm_->GetGlobalWindowMode(DEFAULT_DISPLAY_ID, globalWinMode3);
    EXPECT_EQ(static_cast<uint32_t>(globalWinMode3), 14);

    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: IsSessionInSpecificDisplay01
 * @tc.desc: test IsSessionInSpecificDisplay01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, IsSessionInSpecificDisplay01, TestSize.Level0)
{
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(
        0, SuperFoldStatus::HALF_FOLDED, { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1648, 2472, 0 });
    bool ret = ssm_->IsSessionInSpecificDisplay(nullptr, DEFAULT_DISPLAY_ID);
    EXPECT_EQ(ret, false);

    SessionInfo sessionInfo1;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    WSRect rect1 = { 0, 0, 100, 100 };
    sceneSession1->SetSessionRect(rect1);
    sceneSession1->SetSessionGlobalRect(rect1);
    constexpr DisplayId VIRTUAL_DISPLAY_ID = 999;
    bool ret1 = ssm_->IsSessionInSpecificDisplay(sceneSession1, VIRTUAL_DISPLAY_ID);
    EXPECT_EQ(ret1, false);

    SessionInfo sessionInfo2;
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    WSRect rect2 = { 0, 1700, 100, 100 };
    sceneSession2->SetSessionRect(rect2);
    sceneSession2->SetSessionGlobalRect(rect2);
    bool ret2 = ssm_->IsSessionInSpecificDisplay(sceneSession2, DEFAULT_DISPLAY_ID);
    EXPECT_EQ(ret2, false);

    SessionInfo sessionInfo3;
    sptr<SceneSession> sceneSession3 = sptr<SceneSession>::MakeSptr(sessionInfo3, nullptr);
    WSRect rect3 = { 0, 200, 1200, 1000 };
    sceneSession3->SetSessionRect(rect3);
    sceneSession3->SetSessionGlobalRect(rect3);
    bool ret3 = ssm_->IsSessionInSpecificDisplay(sceneSession3, DEFAULT_DISPLAY_ID);
    EXPECT_EQ(ret3, true);
}

/**
 * @tc.name: HasFloatingWindowForeground01
 * @tc.desc: test HasFloatingWindowForeground with null abilityToken
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, HasFloatingWindowForeground01, TestSize.Level1)
{
    bool hasFloatWindowForeground = false;
    WMError result = ssm_->HasFloatingWindowForeground(nullptr, hasFloatWindowForeground);
    EXPECT_EQ(result, WMError::WM_ERROR_NULLPTR);
    EXPECT_EQ(hasFloatWindowForeground, false);
}

/**
 * @tc.name: ConfigSupportFollowRelativePositionToParent
 * @tc.desc: test ConfigSupportFollowRelativePositionToParent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, ConfigSupportFollowRelativePositionToParent01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    bool ret = true;
    ssm_->ConfigSupportFollowRelativePositionToParent();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: HasFloatingWindowForeground02
 * @tc.desc: test HasFloatingWindowForeground with not existed abilityToken
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, HasFloatingWindowForeground02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.sessionState_ = SessionState::STATE_ACTIVE;
    sessionInfo.persistentId_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);

    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sptr<IRemoteObject> token1 = sptr<MockIRemoteObject>::MakeSptr();
    sceneSession->SetAbilityToken(token1);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    bool hasFloatWindowForeground = false;
    sptr<IRemoteObject> token2 = sptr<MockIRemoteObject>::MakeSptr();
    WMError result = ssm_->HasFloatingWindowForeground(token2, hasFloatWindowForeground);
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(hasFloatWindowForeground, false);
}

/**
 * @tc.name: HasFloatingWindowForeground03
 * @tc.desc: test HasFloatingWindowForeground with existed foreground float window
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, HasFloatingWindowForeground03, TestSize.Level1)
{
    // create first test sceneSession
    SessionInfo sessionInfo1;
    sessionInfo1.sessionState_ = SessionState::STATE_ACTIVE;
    sessionInfo1.persistentId_ = 1;

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    sptr<IRemoteObject> token1 = sptr<MockIRemoteObject>::MakeSptr();
    sceneSession1->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sceneSession1->SetAbilityToken(token1);
    sceneSession1->SetSessionState(SessionState::STATE_ACTIVE);

    // create second test sceneSession
    SessionInfo sessionInfo2;
    sessionInfo2.sessionState_ = SessionState::STATE_FOREGROUND;
    sessionInfo2.persistentId_ = 2;

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    sptr<IRemoteObject> token2 = sptr<MockIRemoteObject>::MakeSptr();
    sceneSession2->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sceneSession2->SetAbilityToken(token2);
    sceneSession2->SetSessionState(SessionState::STATE_FOREGROUND);

    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });

    bool hasFloatWindowForeground = false;
    WMError result = ssm_->HasFloatingWindowForeground(token1, hasFloatWindowForeground);
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(hasFloatWindowForeground, true);

    hasFloatWindowForeground = false;
    result = ssm_->HasFloatingWindowForeground(token2, hasFloatWindowForeground);
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(hasFloatWindowForeground, true);
}

/**
 * @tc.name: HasFloatingWindowForeground04
 * @tc.desc: test HasFloatingWindowForeground with existed background float window
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, HasFloatingWindowForeground04, TestSize.Level1)
{
    // create first test sceneSession
    SessionInfo sessionInfo1;
    sessionInfo1.sessionState_ = SessionState::STATE_INACTIVE;
    sessionInfo1.persistentId_ = 1;

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    sptr<IRemoteObject> token1 = sptr<MockIRemoteObject>::MakeSptr();
    sceneSession1->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sceneSession1->SetAbilityToken(token1);
    sceneSession1->SetSessionState(SessionState::STATE_INACTIVE);

    // create second test sceneSession
    SessionInfo sessionInfo2;
    sessionInfo2.sessionState_ = SessionState::STATE_BACKGROUND;
    sessionInfo2.persistentId_ = 2;

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    sptr<IRemoteObject> token2 = sptr<MockIRemoteObject>::MakeSptr();
    sceneSession2->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sceneSession2->SetAbilityToken(token2);
    sceneSession2->SetSessionState(SessionState::STATE_BACKGROUND);

    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });

    bool hasFloatWindowForeground = false;
    WMError result = ssm_->HasFloatingWindowForeground(token1, hasFloatWindowForeground);
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(hasFloatWindowForeground, false);

    hasFloatWindowForeground = false;
    result = ssm_->HasFloatingWindowForeground(token2, hasFloatWindowForeground);
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(hasFloatWindowForeground, false);
}

/**
 * @tc.name: HasFloatingWindowForeground05
 * @tc.desc: test HasFloatingWindowForeground with existed forground toast window
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, HasFloatingWindowForeground05, TestSize.Level1)
{
    // create first test sceneSession
    SessionInfo sessionInfo;
    sessionInfo.sessionState_ = SessionState::STATE_INACTIVE;
    sessionInfo.persistentId_ = 1;

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sptr<IRemoteObject> token = sptr<MockIRemoteObject>::MakeSptr();
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sceneSession->SetAbilityToken(token);
    sceneSession->SetSessionState(SessionState::STATE_INACTIVE);

    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    bool hasFloatWindowForeground = false;
    WMError result = ssm_->HasFloatingWindowForeground(token, hasFloatWindowForeground);
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(hasFloatWindowForeground, false);
}

/**
 * @tc.name: HasFloatingWindowForeground06
 * @tc.desc: test HasFloatingWindowForeground with other foreground float window
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, HasFloatingWindowForeground06, TestSize.Level1)
{
    // create first test sceneSession
    SessionInfo sessionInfo1;
    sessionInfo1.sessionState_ = SessionState::STATE_ACTIVE;
    sessionInfo1.persistentId_ = 1;

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    sptr<IRemoteObject> token1 = sptr<MockIRemoteObject>::MakeSptr();
    sceneSession1->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sceneSession1->SetAbilityToken(token1);
    sceneSession1->SetSessionState(SessionState::STATE_ACTIVE);

    // create second test sceneSession
    SessionInfo sessionInfo2;
    sessionInfo2.sessionState_ = SessionState::STATE_BACKGROUND;
    sessionInfo2.persistentId_ = 2;

    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo2, nullptr);
    sptr<IRemoteObject> token2 = sptr<MockIRemoteObject>::MakeSptr();
    sceneSession2->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sceneSession2->SetAbilityToken(token2);
    sceneSession2->SetSessionState(SessionState::STATE_BACKGROUND);

    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });

    bool hasFloatWindowForeground = false;
    WMError result = ssm_->HasFloatingWindowForeground(token2, hasFloatWindowForeground);
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(hasFloatWindowForeground, false);
}

/**
 * @tc.name: UpdateHighlightStatus
 * @tc.desc: UpdateHighlightStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, UpdateHighlightStatus, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "UpdateHighlightStatus";
    info.bundleName_ = "UpdateHighlightStatus";
    sptr<SceneSession> preSceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> currSceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    preSceneSession->property_->SetPersistentId(1);
    currSceneSession->property_->SetPersistentId(2);

    sptr<SceneSession> nullSceneSession1;
    sptr<SceneSession> nullSceneSession2;

    ssm_->UpdateHighlightStatus(DEFAULT_DISPLAY_ID, nullSceneSession1, nullSceneSession2, false);
    ssm_->UpdateHighlightStatus(DEFAULT_DISPLAY_ID, preSceneSession, nullSceneSession2, false);
    ssm_->UpdateHighlightStatus(DEFAULT_DISPLAY_ID, preSceneSession, currSceneSession, true);
    ssm_->UpdateHighlightStatus(DEFAULT_DISPLAY_ID, preSceneSession, currSceneSession, false);
    currSceneSession->property_->isExclusivelyHighlighted_ = false;
    preSceneSession->property_->SetPersistentId(2);
    ssm_->UpdateHighlightStatus(DEFAULT_DISPLAY_ID, preSceneSession, currSceneSession, false);
}

/**
 * @tc.name: UpdateHighlightStatus01
 * @tc.desc: UpdateHighlightStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, UpdateHighlightStatus01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->highlightIds_.clear();
    SessionInfo info;
    info.abilityName_ = "UpdateHighlightStatus01";
    info.bundleName_ = "UpdateHighlightStatus01";
    sptr<SceneSession> preSceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> currSceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    preSceneSession->property_->SetPersistentId(1);
    currSceneSession->property_->SetPersistentId(2);
    currSceneSession->property_->isExclusivelyHighlighted_ = false;
    currSceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->UpdateHighlightStatus(DEFAULT_DISPLAY_ID, preSceneSession, currSceneSession, false);
    ASSERT_EQ(ssm_->highlightIds_.size(), 1);
    currSceneSession->property_->SetWindowType(WindowType::BELOW_APP_SYSTEM_WINDOW_BASE);
    currSceneSession->property_->SetPersistentId(3);
    ssm_->UpdateHighlightStatus(DEFAULT_DISPLAY_ID, preSceneSession, currSceneSession, false);
    ASSERT_EQ(ssm_->highlightIds_.size(), 2);
}

/**
 * @tc.name: SetHighlightSessionIds
 * @tc.desc: SetHighlightSessionIds
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, SetHighlightSessionIds, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    SessionInfo info1;
    info1.abilityName_ = "abilityName_test1";
    info1.bundleName_ = "bundleName_test1";

    sptr<SceneSession> currSceneSession = sptr<SceneSession>::MakeSptr(info1, nullptr);
    currSceneSession->property_ = property;
    currSceneSession->property_->SetPersistentId(1);
    currSceneSession->persistentId_ = 1;
    ssm_->highlightIds_.clear();
    ssm_->SetHighlightSessionIds(currSceneSession, false);
    ASSERT_EQ(ssm_->highlightIds_.count(1) == 1, true);
}

/**
 * @tc.name: AddHighlightSessionIds
 * @tc.desc: AddHighlightSessionIds
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, AddHighlightSessionIds, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<WindowSessionProperty> property1 = sptr<WindowSessionProperty>::MakeSptr();
    sptr<WindowSessionProperty> property2 = sptr<WindowSessionProperty>::MakeSptr();

    SessionInfo info1;
    info1.abilityName_ = "abilityName_test1";
    info1.bundleName_ = "bundleName_test1";

    SessionInfo info2;
    info2.abilityName_ = "abilityName_test2";
    info2.bundleName_ = "bundleName_test2";

    sptr<SceneSession> preSceneSession = sptr<SceneSession>::MakeSptr(info1, nullptr);
    sptr<SceneSession> currSceneSession = sptr<SceneSession>::MakeSptr(info2, nullptr);

    preSceneSession->property_->SetPersistentId(1);
    currSceneSession->property_->SetPersistentId(2);
    preSceneSession->persistentId_ = 1;
    currSceneSession->persistentId_ = 2;
    preSceneSession->property_ = property1;
    currSceneSession->property_ = property2;
    ssm_->AddHighlightSessionIds(currSceneSession, false);
    ssm_->AddHighlightSessionIds(preSceneSession, false);
    ASSERT_EQ(ssm_->highlightIds_.count(1) == 1, true);
    ASSERT_EQ(ssm_->highlightIds_.count(2) == 1, true);
}

/**
 * @tc.name: RemoveHighlightSessionIds
 * @tc.desc: RemoveHighlightSessionIds
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, RemoveHighlightSessionIds, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<WindowSessionProperty> property1 = sptr<WindowSessionProperty>::MakeSptr();
    sptr<WindowSessionProperty> property2 = sptr<WindowSessionProperty>::MakeSptr();

    SessionInfo info1;
    info1.abilityName_ = "abilityName_test1";
    info1.bundleName_ = "bundleName_test1";

    SessionInfo info2;
    info2.abilityName_ = "abilityName_test2";
    info2.bundleName_ = "bundleName_test2";

    sptr<SceneSession> preSceneSession = sptr<SceneSession>::MakeSptr(info1, nullptr);
    sptr<SceneSession> currSceneSession = sptr<SceneSession>::MakeSptr(info2, nullptr);

    preSceneSession->property_->SetPersistentId(1);
    currSceneSession->property_->SetPersistentId(2);

    preSceneSession->persistentId_ = 1;
    currSceneSession->persistentId_ = 2;

    preSceneSession->property_ = property1;
    currSceneSession->property_ = property2;
    ssm_->AddHighlightSessionIds(currSceneSession, false);
    ssm_->AddHighlightSessionIds(preSceneSession, false);
    ASSERT_EQ(ssm_->highlightIds_.count(1) == 1, true);
    ASSERT_EQ(ssm_->highlightIds_.count(2) == 1, true);
    ssm_->RemoveHighlightSessionIds(currSceneSession);
    ASSERT_EQ(ssm_->highlightIds_.count(2) == 0, true);
    ssm_->RemoveHighlightSessionIds(preSceneSession);
    ASSERT_EQ(ssm_->highlightIds_.count(1) == 0, true);
}

/**
 * @tc.name: UpdateSessionWithFoldStateChange
 * @tc.desc: test function : UpdateSessionWithFoldStateChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, UpdateSessionWithFoldStateChange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test1";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSessionMocker> sceneSession = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    EXPECT_CALL(*sceneSession, UpdateCrossAxis()).Times(1);
    ssm_->UpdateSessionWithFoldStateChange(10, SuperFoldStatus::EXPANDED, SuperFoldStatus::FOLDED);
}

/**
 * @tc.name: RemoveLifeCycleTaskByPersistentId
 * @tc.desc: test RemoveLifeCycleTaskByPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, RemoveLifeCycleTaskByPersistentId, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "testAbilityName1";
    info.moduleName_ = "testModleName1";
    info.bundleName_ = "testBundleName1";
    info.persistentId_ = 100;

    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.emplace(100, sceneSession);

    auto task = []() {};
    sceneSession->PostLifeCycleTask(task, "task1", LifeCycleTaskType::START);
    ASSERT_EQ(sceneSession->lifeCycleTaskQueue_.size(), 1);
    ssm_->RemoveLifeCycleTaskByPersistentId(100, LifeCycleTaskType::START);
    ASSERT_EQ(sceneSession->lifeCycleTaskQueue_.size(), 0);

    sceneSession->PostLifeCycleTask(task, "task1", LifeCycleTaskType::START);
    ASSERT_EQ(sceneSession->lifeCycleTaskQueue_.size(), 1);
    ssm_->RemoveLifeCycleTaskByPersistentId(3, LifeCycleTaskType::START);
    ASSERT_EQ(sceneSession->lifeCycleTaskQueue_.size(), 1);
}

/**
 * @tc.name: SetStatusBarAvoidHeight
 * @tc.desc: test function : SetStatusBarAvoidHeight
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, SetStatusBarAvoidHeight, TestSize.Level1)
{
    int32_t height = 10;
    ssm_->SetStatusBarAvoidHeight(height);
    WSRect barArea;
    ssm_->GetStatusBarAvoidHeight(barArea);
    ASSERT_EQ(barArea.height_, height);
}

/**
 * @tc.name: GetKeyboardOccupiedAreaWithRotation1
 * @tc.desc: PC device is not compatible
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetKeyboardOccupiedAreaWithRotation1, TestSize.Level1)
{
    GetKeyboardOccupiedAreaWithRotationTestData(
        WindowUIType::PHONE_WINDOW, 0, 0, SessionState::STATE_FOREGROUND, WindowGravity::WINDOW_GRAVITY_BOTTOM);
    ssm_->GetKeyboardOccupiedAreaWithRotation(1, Rotation::ROTATION_0, avoidAreas_);
    ASSERT_EQ(1, static_cast<uint32_t>(avoidAreas_.size()));
    ASSERT_EQ(true, avoidAreas_[0].first);
    ASSERT_EQ(true, CheckKeyboardOccupiedAreaInfo(portraitPanelRect_, avoidAreas_[0].second));

    GetKeyboardOccupiedAreaWithRotationTestData(
        WindowUIType::PAD_WINDOW, 0, 0, SessionState::STATE_FOREGROUND, WindowGravity::WINDOW_GRAVITY_BOTTOM);
    ssm_->GetKeyboardOccupiedAreaWithRotation(1, Rotation::ROTATION_90, avoidAreas_);
    ASSERT_EQ(1, static_cast<uint32_t>(avoidAreas_.size()));
    ASSERT_EQ(true, avoidAreas_[0].first);
    ASSERT_EQ(true, CheckKeyboardOccupiedAreaInfo(landscapePanelRect_, avoidAreas_[0].second));

    GetKeyboardOccupiedAreaWithRotationTestData(
        WindowUIType::PC_WINDOW, 0, 0, SessionState::STATE_FOREGROUND, WindowGravity::WINDOW_GRAVITY_BOTTOM);
    ssm_->GetKeyboardOccupiedAreaWithRotation(1, Rotation::ROTATION_0, avoidAreas_);
    ASSERT_EQ(0, static_cast<uint32_t>(avoidAreas_.size()));
}

/**
 * @tc.name: GetKeyboardOccupiedAreaWithRotation2
 * @tc.desc: test function : GetKeyboardOccupiedAreaWithRotation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetKeyboardOccupiedAreaWithRotation2, TestSize.Level1)
{
    GetKeyboardOccupiedAreaWithRotationTestData(
        WindowUIType::PHONE_WINDOW, 0, 0, SessionState::STATE_FOREGROUND, WindowGravity::WINDOW_GRAVITY_BOTTOM);

    ssm_->GetKeyboardOccupiedAreaWithRotation(-1, Rotation::ROTATION_0, avoidAreas_);
    ASSERT_EQ(0, static_cast<uint32_t>(avoidAreas_.size()));

    ssm_->GetKeyboardOccupiedAreaWithRotation(0, Rotation::ROTATION_0, avoidAreas_);
    ASSERT_EQ(0, static_cast<uint32_t>(avoidAreas_.size()));

    GetKeyboardOccupiedAreaWithRotationTestData(
        WindowUIType::PHONE_WINDOW, 12, 0, SessionState::STATE_FOREGROUND, WindowGravity::WINDOW_GRAVITY_BOTTOM);
    ssm_->GetKeyboardOccupiedAreaWithRotation(1, Rotation::ROTATION_0, avoidAreas_);
    ASSERT_EQ(0, static_cast<uint32_t>(avoidAreas_.size()));
}

/**
 * @tc.name: GetKeyboardOccupiedAreaWithRotation4
 * @tc.desc: test function : GetKeyboardOccupiedAreaWithRotation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetKeyboardOccupiedAreaWithRotation4, TestSize.Level1)
{
    GetKeyboardOccupiedAreaWithRotationTestData(
        WindowUIType::PHONE_WINDOW, 0, 0, SessionState::STATE_FOREGROUND, WindowGravity::WINDOW_GRAVITY_FLOAT);

    ssm_->GetKeyboardOccupiedAreaWithRotation(1, Rotation::ROTATION_0, avoidAreas_);
    ASSERT_EQ(1, static_cast<uint32_t>(avoidAreas_.size()));
    ASSERT_EQ(false, avoidAreas_[0].first);
    ASSERT_EQ(true, CheckKeyboardOccupiedAreaInfo(portraitPanelRect_, avoidAreas_[0].second));
}

/**
 * @tc.name: GetKeyboardOccupiedAreaWithRotation5
 * @tc.desc: test function : GetKeyboardOccupiedAreaWithRotation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetKeyboardOccupiedAreaWithRotation5, TestSize.Level1)
{
    GetKeyboardOccupiedAreaWithRotationTestData(
        WindowUIType::PHONE_WINDOW, 0, 0, SessionState::STATE_BACKGROUND, WindowGravity::WINDOW_GRAVITY_BOTTOM);

    ssm_->GetKeyboardOccupiedAreaWithRotation(1, Rotation::ROTATION_90, avoidAreas_);
    ASSERT_EQ(1, static_cast<uint32_t>(avoidAreas_.size()));
    ASSERT_EQ(false, avoidAreas_[0].first);
    ASSERT_EQ(true, CheckKeyboardOccupiedAreaInfo(landscapePanelRect_, avoidAreas_[0].second));
}

/**
 * @tc.name: GetKeyboardOccupiedAreaWithRotation6
 * @tc.desc: test function : GetKeyboardOccupiedAreaWithRotation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetKeyboardOccupiedAreaWithRotation6, TestSize.Level1)
{
    GetKeyboardOccupiedAreaWithRotationTestData(
        WindowUIType::PHONE_WINDOW, 0, 0, SessionState::STATE_FOREGROUND, WindowGravity::WINDOW_GRAVITY_BOTTOM);

    ssm_->GetKeyboardOccupiedAreaWithRotation(1, Rotation::ROTATION_180, avoidAreas_);
    ASSERT_EQ(1, static_cast<uint32_t>(avoidAreas_.size()));
    ASSERT_EQ(true, avoidAreas_[0].first);
    ASSERT_EQ(true, CheckKeyboardOccupiedAreaInfo(portraitPanelRect_, avoidAreas_[0].second));

    ssm_->GetKeyboardOccupiedAreaWithRotation(1, Rotation::ROTATION_0, avoidAreas_);
    ASSERT_EQ(2, static_cast<uint32_t>(avoidAreas_.size()));
    ASSERT_EQ(true, avoidAreas_[1].first);
    ASSERT_EQ(true, CheckKeyboardOccupiedAreaInfo(portraitPanelRect_, avoidAreas_[1].second));
}

/**
 * @tc.name: GetKeyboardOccupiedAreaWithRotation7
 * @tc.desc: test function : GetKeyboardOccupiedAreaWithRotation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetKeyboardOccupiedAreaWithRotation7, TestSize.Level1)
{
    GetKeyboardOccupiedAreaWithRotationTestData(
        WindowUIType::PHONE_WINDOW, 0, 0, SessionState::STATE_ACTIVE, WindowGravity::WINDOW_GRAVITY_BOTTOM);
    ssm_->GetKeyboardOccupiedAreaWithRotation(1, Rotation::ROTATION_270, avoidAreas_);

    ASSERT_EQ(1, static_cast<uint32_t>(avoidAreas_.size()));
    ASSERT_EQ(true, avoidAreas_[0].first);
    ASSERT_EQ(true, CheckKeyboardOccupiedAreaInfo(landscapePanelRect_, avoidAreas_[0].second));
}

/**
 * @tc.name: SetFocusedSessionDisplayIdIfNeededTest
 * @tc.desc: test function : SetFocusedSessionDisplayIdIfNeeded
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, SetFocusedSessionDisplayIdIfNeededTest, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test1";
    sptr<SceneSessionMocker> sceneSessionMock = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetDisplayId(DISPLAY_ID_INVALID);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSessionMock->SetSessionProperty(property);

    EXPECT_CALL(*sceneSessionMock, SetScreenId(_)).Times(1);
    sptr<SceneSession> sceneSession = static_cast<sptr<SceneSession>>(sceneSessionMock);
    ssm_->SetFocusedSessionDisplayIdIfNeeded(sceneSession);
}

/**
 * @tc.name: SetFocusedSessionDisplayIdIfNeededTest001
 * @tc.desc: test function : SetFocusedSessionDisplayIdIfNeeded
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, SetFocusedSessionDisplayIdIfNeededTest001, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test1";
    sptr<SceneSessionMocker> sceneSessionMock = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetDisplayId(DISPLAY_ID_INVALID + 1);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSessionMock->SetSessionProperty(property);

    EXPECT_CALL(*sceneSessionMock, SetScreenId(_)).Times(0);
    sptr<SceneSession> sceneSession = static_cast<sptr<SceneSession>>(sceneSessionMock);
    ssm_->SetFocusedSessionDisplayIdIfNeeded(sceneSession);
}

/**
 * @tc.name: QueryAbilityInfoFromBMSTest
 * @tc.desc: SceneSesionManager QueryAbilityInfoFromBMS NotifyStartAbility
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, QueryAbilityInfoFromBMSTest, TestSize.Level1)
{
    const int32_t uId = 32;
    SessionInfo sessionInfo_;
    sessionInfo_.bundleName_ = "BundleName";
    sessionInfo_.abilityName_ = "AbilityName";
    sessionInfo_.moduleName_ = "ModuleName";
    ssm_->bundleMgr_ = nullptr;

    auto res = ssm_->QueryAbilityInfoFromBMS(
        uId, sessionInfo_.bundleName_, sessionInfo_.abilityName_, sessionInfo_.moduleName_);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: QueryAbilityInfoFromBMSTest001
 * @tc.desc: SceneSesionManager QueryAbilityInfoFromBMS NotifyStartAbility
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, QueryAbilityInfoFromBMSTest001, TestSize.Level1)
{
    const int32_t uId = 32;
    SessionInfo sessionInfo_;
    sessionInfo_.bundleName_ = "BundleName";
    sessionInfo_.abilityName_ = "AbilityName";
    sessionInfo_.moduleName_ = "ModuleName";
    ssm_->bundleMgr_ = ssm_->GetBundleManager();
    SceneSessionManager::SessionInfoList listKey = {
        .uid_ = uId, .bundleName_ = "BundleName", .abilityName_ = "AbilityName", .moduleName_ = "ModuleName"
    };
    ssm_->abilityInfoMap_[listKey] = std::make_shared<AppExecFwk::AbilityInfo>();

    auto res = ssm_->QueryAbilityInfoFromBMS(
        uId, sessionInfo_.bundleName_, sessionInfo_.abilityName_, sessionInfo_.moduleName_);
    EXPECT_NE(res, nullptr);
}

/**
 * @tc.name: RequestFocusSpecificCheckTest
 * @tc.desc: Test for RequestFocusSpecificCheck
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, RequestFocusSpecificCheckTest, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    bool byForeground = true;
    FocusChangeReason reason = FocusChangeReason::CLIENT_REQUEST;

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    session->persistentId_ = 1;
    sceneSession->dialogVec_.push_back(session);
    ssm_->windowFocusController_->UpdateFocusedSessionId(DEFAULT_DISPLAY_ID, 1);
    sceneSession->SetForceHideState(ForceHideState::NOT_HIDDEN);

    WSError result = ssm_->RequestFocusSpecificCheck(DEFAULT_DISPLAY_ID, sceneSession, byForeground, reason);
    EXPECT_EQ(result, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: NotifyUnFocusedByMissionTest001
 * @tc.desc: Test for NotifyUnFocusedByMission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, NotifyUnFocusedByMissionTest001, TestSize.Level1)
{
    sptr<SceneSession> sceneSession;
    ssm_->NotifyUnFocusedByMission(sceneSession);
    EXPECT_EQ(sceneSession, nullptr);
}

/**
 * @tc.name: NotifyUnFocusedByMissionTest002
 * @tc.desc: Test for NotifyUnFocusedByMission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, NotifyUnFocusedByMissionTest002, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "NotifyUnFocusedByMission";
    info.abilityName_ = "NotifyUnFocusedByMission";
    info.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->NotifyUnFocusedByMission(sceneSession);
    EXPECT_EQ(sceneSession->GetSessionInfo().isSystem_, true);
}

/**
 * @tc.name: NotifyUnFocusedByMissionTest003
 * @tc.desc: Test for NotifyUnFocusedByMission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, NotifyUnFocusedByMissionTest003, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "NotifyUnFocusedByMission";
    info.abilityName_ = "NotifyUnFocusedByMission";
    info.isSystem_ = false;
    ssm_->listenerController_ = std::make_shared<SessionListenerController>();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->NotifyUnFocusedByMission(sceneSession);
    EXPECT_EQ(sceneSession->GetSessionInfo().isSystem_, false);
}

/**
 * @tc.name: NotifyStackEmptyTest
 * @tc.desc: test function : NotifyStackEmpty
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, NotifyStackEmptyTest, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "NotifyStackEmpty";
    info.abilityName_ = "NotifyStackEmpty";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    int persistentId = 1112;
    sceneSession->property_->SetPersistentId(persistentId);
    auto ret = ssm_->NotifyStackEmpty(persistentId);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: AddSkipSelfWhenShowOnVirtualScreenList
 * @tc.desc: test function : AddSkipSelfWhenShowOnVirtualScreenList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, AddSkipSelfWhenShowOnVirtualScreenList, Function | SmallTest | Level1)
{
    SessionInfo info;
    info.bundleName_ = "AddSkipSelfWhenShowOnVirtualScreenList";
    info.abilityName_ = "AddSkipSelfWhenShowOnVirtualScreenList";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    std::vector<int32_t> persistentIds{ sceneSession->GetPersistentId() };
    auto ret = ssm_->AddSkipSelfWhenShowOnVirtualScreenList(persistentIds);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: RemoveSkipSelfWhenShowOnVirtualScreenList
 * @tc.desc: test function : RemoveSkipSelfWhenShowOnVirtualScreenList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, RemoveSkipSelfWhenShowOnVirtualScreenList, Function | SmallTest | Level1)
{
    SessionInfo info;
    info.bundleName_ = "RemoveSkipSelfWhenShowOnVirtualScreenList";
    info.abilityName_ = "RemoveSkipSelfWhenShowOnVirtualScreenList";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    std::vector<int32_t> persistentIds{ sceneSession->GetPersistentId() };
    auto ret = ssm_->RemoveSkipSelfWhenShowOnVirtualScreenList(persistentIds);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: GetCallingWindowInfo1
 * @tc.desc: test function : GetCallingWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetCallingWindowInfo1, TestSize.Level1)
{
    KeyboardTestData keyboardTestData(0, 57256, 0, WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT, false);
    keyboardTestData.SetCallingSessionId(86);
    ConstructKeyboardCallingWindowTestData(keyboardTestData);
    // Invalid callingWindowId
    CallingWindowInfo info(0, -1, 0, GetUserIdByUid(getuid())); // windowId_ callingPid_ displayId_ userId_
    WMError ret = ssm_->GetCallingWindowInfo(info);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: GetCallingWindowInfo2
 * @tc.desc: test function : GetCallingWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetCallingWindowInfo2, TestSize.Level1)
{
    KeyboardTestData keyboardTestData(0, 57256, 0, WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT, false);
    keyboardTestData.SetCallingSessionId(86);
    ConstructKeyboardCallingWindowTestData(keyboardTestData);
    // Invalid userId
    int32_t userId = GetUserIdByUid(getuid()) + 1;
    CallingWindowInfo info(86, -1, 0, userId); // windowId_ callingPid_ displayId_ userId_
    WMError ret = ssm_->GetCallingWindowInfo(info);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: GetCallingWindowInfo3
 * @tc.desc: test function : GetCallingWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetCallingWindowInfo3, TestSize.Level1)
{
    KeyboardTestData keyboardTestData(12, 57256, 0, WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT, false);
    keyboardTestData.SetCallingSessionId(86);
    ConstructKeyboardCallingWindowTestData(keyboardTestData);
    int32_t userId = GetUserIdByUid(getuid());
    CallingWindowInfo info(86, -1, 0, userId); // windowId_ callingPid_ displayId_ userId_
    WMError ret = ssm_->GetCallingWindowInfo(info);
    ASSERT_EQ(WMError::WM_OK, ret);
    ASSERT_EQ(57256, info.callingPid_);
    ASSERT_EQ(12, info.displayId_);
}

/**
 * @tc.name: UpdateSessionDisplayId1
 * @tc.desc: test function : UpdateSessionDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, UpdateSessionDisplayId1, TestSize.Level0)
{
    sptr<WindowManagerAgentLiteMocker> wmAgentLiteMocker = sptr<WindowManagerAgentLiteMocker>::MakeSptr();
    SessionManagerAgentController::GetInstance().RegisterWindowManagerAgent(
        wmAgentLiteMocker, WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CALLING_DISPLAY, 12345);
    KeyboardTestData keyboardTestData(0, 57256, 0, WindowType::WINDOW_TYPE_APP_SUB_WINDOW, false);
    keyboardTestData.SetCallingSessionId(86);
    ConstructKeyboardCallingWindowTestData(keyboardTestData);
    EXPECT_CALL(*wmAgentLiteMocker, NotifyCallingWindowDisplayChanged(_)).Times(0);
    ssm_->UpdateSessionDisplayId(86, 12);

    keyboardTestData = { 0, 57256, 0, WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT, true };
    keyboardTestData.SetCallingSessionId(86);
    ConstructKeyboardCallingWindowTestData(keyboardTestData);
    EXPECT_CALL(*wmAgentLiteMocker, NotifyCallingWindowDisplayChanged(_)).Times(0);
    ssm_->UpdateSessionDisplayId(86, 12);
}

/**
 * @tc.name: UpdateSessionDisplayId2
 * @tc.desc: test function : UpdateSessionDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, UpdateSessionDisplayId2, TestSize.Level0)
{
    sptr<WindowManagerAgentLiteMocker> wmAgentLiteMocker = sptr<WindowManagerAgentLiteMocker>::MakeSptr();
    SessionManagerAgentController::GetInstance().RegisterWindowManagerAgent(
        wmAgentLiteMocker, WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CALLING_DISPLAY, 12345);
    KeyboardTestData keyboardTestData(0, 57256, 0, WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT, false);
    keyboardTestData.SetCallingSessionId(86);
    ConstructKeyboardCallingWindowTestData(keyboardTestData);
    SessionInfo info;
    info.abilityName_ = "non-callingSession";
    info.bundleName_ = "non-callingSession";
    info.screenId_ = 0;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->callingPid_ = 54321;
    sptr<WindowSessionProperty> sceneSessionProperties = sptr<WindowSessionProperty>::MakeSptr();
    sceneSessionProperties->SetDisplayId(0);
    sceneSession->SetSessionProperty(sceneSessionProperties);
    // Add non-callingWindow
    ssm_->sceneSessionMap_.insert({ 90, sceneSession });
    // Change display id of non-callingWindow
    EXPECT_CALL(*wmAgentLiteMocker, NotifyCallingWindowDisplayChanged(_)).Times(0);
    ssm_->UpdateSessionDisplayId(90, 12);
}

/**
 * @tc.name: UpdateSessionDisplayId3
 * @tc.desc: test function : UpdateSessionDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, UpdateSessionDisplayId3, TestSize.Level1)
{
    sptr<WindowManagerAgentLiteMocker> wmAgentLiteMocker = sptr<WindowManagerAgentLiteMocker>::MakeSptr();
    SessionManagerAgentController::GetInstance().RegisterWindowManagerAgent(
        wmAgentLiteMocker, WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_CALLING_DISPLAY, 12345);
    KeyboardTestData keyboardTestData(0, 57256, 0, WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT, false);
    keyboardTestData.SetCallingSessionId(86);
    ConstructKeyboardCallingWindowTestData(keyboardTestData);
    EXPECT_CALL(*wmAgentLiteMocker, NotifyCallingWindowDisplayChanged(_)).Times(1);
    ssm_->UpdateSessionDisplayId(86, 12);
}

/**
 * @tc.name: GetActiveSceneSessionCopy
 * @tc.desc: test function : GetActiveSceneSessionCopy
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetActiveSceneSessionCopy, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "GetActiveSceneSessionCopy";
    info.bundleName_ = "GetActiveSceneSessionCopy";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSessionMocker> sceneSession = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    std::vector<sptr<SceneSession>> activeSession = ssm_->GetActiveSceneSessionCopy();
    EXPECT_EQ(activeSession.empty(), false);
}

/**
 * @tc.name: GetHookedSessionByModuleName
 * @tc.desc: test function : GetHookedSessionByModuleName
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetHookedSessionByModuleName, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.bundleName_ = "testBundleName1";
    info.moduleName_ = "testModuleName1";
    info.appIndex_ = 1;
    info.appInstanceKey_ = "";
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    auto res = ssm_->GetHookedSessionByModuleName(info);
    ASSERT_EQ(res, nullptr);

    ssm_->sceneSessionMap_.insert({ 101, sceneSession });
    res = ssm_->GetHookedSessionByModuleName(info);
    ASSERT_EQ(res, sceneSession);

    info.appInstanceKey_ = "testAppInstanceKey1";
    res = ssm_->GetHookedSessionByModuleName(info);
    ASSERT_EQ(res, nullptr);

    info.appIndex_ = 2;
    res = ssm_->GetHookedSessionByModuleName(info);
    ASSERT_EQ(res, nullptr);

    info.moduleName_ = "testModuleName2";
    res = ssm_->GetHookedSessionByModuleName(info);
    ASSERT_EQ(res, nullptr);

    info.bundleName_ = "testBundleName2";
    res = ssm_->GetHookedSessionByModuleName(info);
    ASSERT_EQ(res, nullptr);
}

/**
 * @tc.name: RequestSceneSession
 * @tc.desc: test function : RequestSceneSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, RequestSceneSession, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.bundleName_ = "request_scene_session_bundle";
    info.moduleName_ = "request_scene_session_module";
    info.abilityName_ = "request_scene_session_ability";
    info.persistentId_ = 101;
    info.appIndex_ = 0;
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->sessionInfo_.isAbilityHook_ = true;
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    ssm_->sceneSessionMap_[101] = sceneSession;

    auto result = ssm_->RequestSceneSession(info, windowSessionProperty);
    ASSERT_NE(result, nullptr);
    ASSERT_EQ(result->GetSessionInfo().moduleName_, info.moduleName_);
}

/**
 * @tc.name: UpdateAbilityHookState
 * @tc.desc: test function : UpdateAbilityHookState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, UpdateAbilityHookState, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.bundleName_ = "UpdateAbilityHookState_bundle";
    info.moduleName_ = "UpdateAbilityHookState_module";
    info.abilityName_ = "UpdateAbilityHookState_ability";
    info.persistentId_ = 101;
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    ssm_->UpdateAbilityHookState(sceneSession, true);
    EXPECT_EQ(true, sceneSession->GetSessionInfo().isAbilityHook_);
}

/**
 * @tc.name: UpdateRecentMainSessionInfos
 * @tc.desc: test function : UpdateRecentMainSessionInfos
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, UpdateRecentMainSessionInfos, Function | SmallTest | Level2)
{
    const std::vector<int32_t> recentMainSessionIdList = { 101 };
    SessionInfo info;
    info.bundleName_ = "UpdateRecentMainSessionInfoList BundleName";
    info.moduleName_ = "UpdateRecentMainSessionInfoList ModuleName";
    info.abilityName_ = "UpdateRecentMainSessionInfoList AbilityName";
    info.persistentId_ = 101;
    info.appIndex_ = 0;
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->sceneSessionMap_[101] = sceneSession;
    ssm_->recentMainSessionInfoList_.clear();
    EXPECT_EQ(ssm_->recentMainSessionInfoList_.size(), 0);
    ssm_->UpdateRecentMainSessionInfos(recentMainSessionIdList);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(ssm_->recentMainSessionInfoList_.size(), 1);
}

/**
 * @tc.name: GetRecentMainSessionInfoList
 * @tc.desc: test function : GetRecentMainSessionInfoList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest12, GetRecentMainSessionInfoList, Function | SmallTest | Level2)
{
    std::vector<RecentSessionInfo> recentSessionInfoList = {};
    auto result = ssm_->GetRecentMainSessionInfoList(recentSessionInfoList);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PERMISSION);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
