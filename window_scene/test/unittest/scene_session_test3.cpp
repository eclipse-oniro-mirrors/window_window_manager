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

#include "application_context.h"
#include "display_manager.h"
#include "input_event.h"
#include "key_event.h"
#include "mock/mock_session_stage.h"
#include "pointer_event.h"

#include "session/host/include/main_session.h"
#include "session/host/include/keyboard_session.h"
#define private public
#define protected public
#include "session/host/include/scene_session.h"
#undef private
#undef protected
#include "session/host/include/sub_session.h"
#include "session/host/include/system_session.h"
#include "ui/rs_surface_node.h"
#include "window_helper.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace {
    std::string logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        logMsg = msg;
    }
}

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t SLEEP_TIME_US = 100000; // 100ms
}
class SceneSessionTest3 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneSessionTest3::SetUpTestCase() {}

void SceneSessionTest3::TearDownTestCase() {}

void SceneSessionTest3::SetUp() {}

void SceneSessionTest3::TearDown() {}

namespace {
/**
 * @tc.name: NotifyClientToUpdateRectTask
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, NotifyClientToUpdateRectTask, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyClientToUpdateRectTask";
    info.bundleName_ = "NotifyClientToUpdateRectTask";
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);

    sceneSession->SetSessionProperty(property);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    auto result = sceneSession->NotifyClientToUpdateRectTask("SceneSessionTest3", nullptr);
    ASSERT_EQ(result, WSError::WS_OK);

    property->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    sceneSession->SetSessionProperty(property);
    sceneSession->isKeyboardPanelEnabled_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->isScbCoreEnabled_ = false;
    sceneSession->SetSessionRect({ 0, 0, 800, 800 });
    EXPECT_EQ(WSError::WS_OK, sceneSession->NotifyClientToUpdateRectTask("SceneSessionTest3", nullptr));

    property->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    sceneSession->SetSessionProperty(property);
    sceneSession->isKeyboardPanelEnabled_ = true;
    EXPECT_EQ(WSError::WS_OK, sceneSession->NotifyClientToUpdateRectTask("SceneSessionTest3", nullptr));

    std::shared_ptr<RSTransaction> rs;
    EXPECT_EQ(WSError::WS_OK, sceneSession->NotifyClientToUpdateRectTask("SceneSessionTest3", rs));

    sceneSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_MOVE);
    EXPECT_EQ(WSError::WS_OK, sceneSession->NotifyClientToUpdateRectTask("SceneSessionTest3", rs));

    sceneSession->UpdateSizeChangeReason(SizeChangeReason::DRAG);
    EXPECT_EQ(WSError::WS_OK, sceneSession->NotifyClientToUpdateRectTask("SceneSessionTest3", rs));

    sceneSession->UpdateSizeChangeReason(SizeChangeReason::ROTATION);
    EXPECT_EQ(WSError::WS_OK, sceneSession->NotifyClientToUpdateRectTask("SceneSessionTest3", rs));
}

/**
 * @tc.name: UpdateCrossPlaneState
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, UpdateCrossPlaneState, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateCrossPlaneState";
    info.bundleName_ = "UpdateCrossPlaneState";
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    WSRect rect = { 0, 0, 800, 800 };
    sceneSession->SetScreenId(0);

    PcFoldScreenManager::GetInstance().displayId_ = 0;
    PcFoldScreenManager::GetInstance().screenFoldStatus_ = SuperFoldStatus::FOLDED;
    PcFoldScreenManager::GetInstance().hasSystemKeyboard_ = false;
    EXPECT_EQ(CrossPlaneState::CROSS_DEFAULT_PLANE, sceneSession->UpdateCrossPlaneState(rect));

    PcFoldScreenManager::GetInstance().screenFoldStatus_ = SuperFoldStatus::HALF_FOLDED;
    PcFoldScreenManager::GetInstance().hasSystemKeyboard_ = true;
    EXPECT_EQ(CrossPlaneState::CROSS_DEFAULT_PLANE, sceneSession->UpdateCrossPlaneState(rect));

    PcFoldScreenManager::GetInstance().SetDisplayRects(
        { 0, 0, 2472, 1739 }, { 0, 1791, 2472, 1648 }, { 0, 1740, 2472, 50 });
    PcFoldScreenManager::GetInstance().hasSystemKeyboard_ = false;

    rect = { 0, 1630, 100, 500 };
    EXPECT_EQ(CrossPlaneState::CROSS_ALL_PLANE, sceneSession->UpdateCrossPlaneState(rect));

    rect = { 0, 0, 100, 100 };
    EXPECT_EQ(CrossPlaneState::CROSS_DEFAULT_PLANE, sceneSession->UpdateCrossPlaneState(rect));

    rect = { 0, 1800, 100, 100 };
    EXPECT_EQ(CrossPlaneState::CROSS_VIRTUAL_PLANE, sceneSession->UpdateCrossPlaneState(rect));

    rect = { 0, 1750, 10, 100 };
    EXPECT_EQ(CrossPlaneState::CROSS_VIRTUAL_CREASE_PLANE, sceneSession->UpdateCrossPlaneState(rect));

    rect = { 0, 1730, 20, 30 };
    EXPECT_EQ(CrossPlaneState::CROSS_DEFAULT_CREASE_PLANE, sceneSession->UpdateCrossPlaneState(rect));

    rect = { 0, 1741, 1, 1 };
    EXPECT_EQ(CrossPlaneState::CROSS_CREASE_PLANE, sceneSession->UpdateCrossPlaneState(rect));
}

/**
 * @tc.name: BindDialogSessionTarget1
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, BindDialogSessionTarget1, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "BindDialogSessionTarget1";
    info.bundleName_ = "BindDialogSessionTarget1";
    sptr<SceneSession::SpecificSessionCallback> specificCallback_ =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCallback_, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->isActive_ = true;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    property->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    sceneSession->SetSessionProperty(property);

    sptr<SceneSession> sceneSession1 = nullptr;
    WSError result = sceneSession->BindDialogSessionTarget(sceneSession1);
    ASSERT_EQ(result, WSError::WS_ERROR_NULLPTR);

    sptr<SceneSession> sceneSession2 = sceneSession;
    result = sceneSession->BindDialogSessionTarget(sceneSession2);
    ASSERT_EQ(result, WSError::WS_OK);

    sceneSession->onBindDialogTarget_ = [](const sptr<SceneSession>&) {};
    sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    result = sceneSession->BindDialogSessionTarget(sceneSession1);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: RegisterBindDialogSessionCallback1
 * @tc.desc: test RegisterBindDialogSessionCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, RegisterBindDialogSessionCallback1, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RegisterBindDialogSessionCallback1";
    info.bundleName_ = "RegisterBindDialogSessionCallback1";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->onBindDialogTarget_ = nullptr;
    NotifyBindDialogSessionFunc func = [](const sptr<SceneSession>& sceneSession) {};
    sceneSession->RegisterBindDialogSessionCallback(func);
    ASSERT_NE(sceneSession->onBindDialogTarget_, nullptr);
}

/**
 * @tc.name: ClearSpecificSessionCbMap1
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, ClearSpecificSessionCbMap1, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ClearSpecificSessionCbMap1";
    info.bundleName_ = "ClearSpecificSessionCbMap1";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    sceneSession->ClearSpecificSessionCbMap();

    sceneSession->clearCallbackMapFunc_ = [](bool) {};
    sceneSession->ClearSpecificSessionCbMap();
}

/**
 * @tc.name: IsMovableWindowType
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, IsMovableWindowType, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsMovableWindowType";
    info.bundleName_ = "IsMovableWindowType";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);

    sceneSession->property_ = nullptr;
    EXPECT_EQ(sceneSession->IsMovableWindowType(), false);
}

/**
 * @tc.name: SetBlank
 * @tc.desc: check func SetBlank
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, SetBlank, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetBlank";
    info.bundleName_ = "SetBlank";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);

    bool isAddBlank = true;
    sceneSession->SetBlank(isAddBlank);
    ASSERT_EQ(isAddBlank, sceneSession->GetBlank());
}

/**
 * @tc.name: GetBlank
 * @tc.desc: check func GetBlank
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, GetBlank, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetBlank";
    info.bundleName_ = "GetBlank";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);

    bool isAddBlank = true;
    sceneSession->SetBlank(isAddBlank);
    ASSERT_EQ(isAddBlank, sceneSession->GetBlank());
}

/**
 * @tc.name: SetBufferAvailableCallbackEnable
 * @tc.desc: check func SetBufferAvailableCallbackEnable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, SetBufferAvailableCallbackEnable, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetBufferAvailableCallbackEnable";
    info.bundleName_ = "SetBufferAvailableCallbackEnable";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);

    bool enable = true;
    sceneSession->SetBufferAvailableCallbackEnable(enable);
    ASSERT_EQ(enable, sceneSession->GetBufferAvailableCallbackEnable());
}

/**
 * @tc.name: GetBufferAvailableCallbackEnable
 * @tc.desc: check func GetBufferAvailableCallbackEnable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, GetBufferAvailableCallbackEnable, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetBufferAvailableCallbackEnable";
    info.bundleName_ = "GetBufferAvailableCallbackEnable";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);

    bool enable = true;
    sceneSession->SetBufferAvailableCallbackEnable(enable);
    ASSERT_EQ(enable, sceneSession->GetBufferAvailableCallbackEnable());
}

/**
 * @tc.name: NotifyClientToUpdateAvoidArea
 * @tc.desc: check func NotifyClientToUpdateAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, NotifyClientToUpdateAvoidArea, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyClientToUpdateAvoidArea";
    info.bundleName_ = "NotifyClientToUpdateAvoidArea";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);

    sceneSession->NotifyClientToUpdateAvoidArea();
    EXPECT_EQ(nullptr, sceneSession->specificCallback_);

    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sceneSession = sptr<SceneSession>::MakeSptr(info, callback);
    EXPECT_NE(nullptr, sceneSession);
    sceneSession->persistentId_ = 6;
    callback->onUpdateAvoidArea_ = nullptr;
    sceneSession->NotifyClientToUpdateAvoidArea();

    UpdateAvoidAreaCallback callbackFun = [&sceneSession](int32_t persistentId) {
        sceneSession->RemoveToastSession(persistentId);
        return;
    };
    callback->onUpdateAvoidArea_ = callbackFun;
    sceneSession->NotifyClientToUpdateAvoidArea();
    EXPECT_EQ(6, sceneSession->GetPersistentId());
}

/**
 * @tc.name: UpdateScaleInner
 * @tc.desc: check func UpdateScaleInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, UpdateScaleInner, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateScaleInner";
    info.bundleName_ = "UpdateScaleInner";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);

    sceneSession->sessionStage_ = nullptr;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    bool res = sceneSession->UpdateScaleInner(10.0f, 10.0f, 10.0f, 10.0f);
    EXPECT_EQ(true, res);

    res = sceneSession->UpdateScaleInner(10.0f, 9.0f, 10.0f, 10.0f);
    res = sceneSession->UpdateScaleInner(10.0f, 9.0f, 9.0f, 10.0f);
    res = sceneSession->UpdateScaleInner(10.0f, 9.0f, 9.0f, 9.0f);
    EXPECT_EQ(true, res);

    sceneSession->state_ = SessionState::STATE_BACKGROUND;
    res = sceneSession->UpdateScaleInner(10.0f, 9.0f, 9.0f, 9.0f);
    EXPECT_EQ(false, res);

    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    sceneSession->sessionStage_ = mockSessionStage;
    res = sceneSession->UpdateScaleInner(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_EQ(true, res);
    res = sceneSession->UpdateScaleInner(1.0f, 2.0f, 3.0f, 4.0f);
    EXPECT_EQ(false, res);
}

/**
 * @tc.name: UpdateZOrderInner
 * @tc.desc: check func UpdateZOrderInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, UpdateZOrderInner, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateZOrderInner";
    info.bundleName_ = "UpdateZOrderInner";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);

    int res = sceneSession->UpdateZOrderInner(10);
    EXPECT_EQ(true, res);

    res = sceneSession->UpdateZOrderInner(10);
    EXPECT_EQ(false, res);
}

/**
 * @tc.name: GetSubWindowModalType02
 * @tc.desc: GetSubWindowModalType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, GetSubWindowModalType02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ModalType2";
    info.bundleName_ = "ModalType2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    property->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_TOAST));
    sceneSession->SetSessionProperty(property);
    auto result = sceneSession->GetSubWindowModalType();
    ASSERT_EQ(result, SubWindowModalType::TYPE_TOAST);
}

/**
 * @tc.name: GetSubWindowModalType03
 * @tc.desc: GetSubWindowModalType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, GetSubWindowModalType03, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ModalType3";
    info.bundleName_ = "ModalType3";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    property->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_MODAL));
    sceneSession->SetSessionProperty(property);
    auto result = sceneSession->GetSubWindowModalType();
    ASSERT_EQ(result, SubWindowModalType::TYPE_WINDOW_MODALITY);
}

/**
 * @tc.name: GetSubWindowModalType04
 * @tc.desc: GetSubWindowModalType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, GetSubWindowModalType04, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ModalType4";
    info.bundleName_ = "ModalType4";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    property->SetWindowFlags(static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_NEED_AVOID));
    sceneSession->SetSessionProperty(property);
    auto result = sceneSession->GetSubWindowModalType();
    ASSERT_EQ(result, SubWindowModalType::TYPE_NORMAL);
}

/**
 * @tc.name: RegisterDefaultAnimationFlagChangeCallback
 * @tc.desc: RegisterDefaultAnimationFlagChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, RegisterDefaultAnimationFlagChangeCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RegisterDefaultAnimationFlagChangeCallback";
    info.bundleName_ = "RegisterDefaultAnimationFlagChangeCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->RegisterDefaultAnimationFlagChangeCallback([sceneSession](const bool flag) { return; });
    ASSERT_EQ(WSError::WS_OK, sceneSession->UpdateWindowAnimationFlag(true));

    sceneSession->RegisterDefaultAnimationFlagChangeCallback([sceneSession](const bool flag) { return; });
    ASSERT_EQ(WSError::WS_OK, sceneSession->UpdateWindowAnimationFlag(true));
}

/**
 * @tc.name: SetMainWindowTopmostChangeCallback
 * @tc.desc: SetMainWindowTopmostChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, SetMainWindowTopmostChangeCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetMainWindowTopmostChangeCallback";
    info.bundleName_ = "SetMainWindowTopmostChangeCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    NotifyMainWindowTopmostChangeFunc func;
    sceneSession->SetMainWindowTopmostChangeCallback(std::move(func));

    NotifyMainWindowTopmostChangeFunc func1 = [sceneSession](bool isTopmost) { return; };
    sceneSession->SetMainWindowTopmostChangeCallback(std::move(func1));
    ASSERT_NE(nullptr, sceneSession->mainWindowTopmostChangeFunc_);
}

/**
 * @tc.name: SetRestoreMainWindowCallback
 * @tc.desc: SetRestoreMainWindowCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, SetRestoreMainWindowCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetRestoreMainWindowCallback";
    info.bundleName_ = "SetRestoreMainWindowCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->SetRestoreMainWindowCallback([](
        bool isAppSupportPhoneInPc, int32_t callingPid, uint32_t callingToken) {
        return;
    });
    ASSERT_NE(nullptr, sceneSession->onRestoreMainWindowFunc_);
}

/**
 * @tc.name: SetAdjustKeyboardLayoutCallback
 * @tc.desc: SetAdjustKeyboardLayoutCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, SetAdjustKeyboardLayoutCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetAdjustKeyboardLayoutCallback";
    info.bundleName_ = "SetAdjustKeyboardLayoutCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->property_ = windowSessionProperty;

    NotifyKeyboardLayoutAdjustFunc func;
    sceneSession->SetAdjustKeyboardLayoutCallback(func);

    NotifyKeyboardLayoutAdjustFunc func1 = [sceneSession](const KeyboardLayoutParams& params) { return; };
    sceneSession->SetAdjustKeyboardLayoutCallback(func1);
    ASSERT_NE(nullptr, sceneSession->adjustKeyboardLayoutFunc_);
}

/**
 * @tc.name: IsStartMoving
 * @tc.desc: get main window move flag, test IsStartMoving
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, IsStartMoving, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsStartMoving";
    info.bundleName_ = "IsStartMoving";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    ASSERT_EQ(false, sceneSession->IsStartMoving());

    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    ASSERT_EQ(false, sceneSession->IsStartMoving());
}

/**
 * @tc.name: SetAppSupportPhoneInPc
 * @tc.desc: SetAppSupportPhoneInPc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, SetAppSupportPhoneInPc, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetAppSupportPhoneInPc";
    info.bundleName_ = "SetAppSupportPhoneInPc";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->property_ = windowSessionProperty;
    ASSERT_EQ(WSError::WS_OK, sceneSession->SetAppSupportPhoneInPc(false));
}

/**
 * @tc.name: SetIsMidScene
 * @tc.desc: SetIsMidScene
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, SetIsMidScene, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetIsMidScene";
    info.bundleName_ = "SetIsMidScene";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    sceneSession->SetIsMidScene(true);
    bool res = sceneSession->GetIsMidScene();
    EXPECT_EQ(res, true);
}

/**
 * @tc.name: SetIsPcAppInPad
 * @tc.desc: SetIsPcAppInPad
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, SetIsPcAppInPad, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetIsPcAppInPad";
    info.bundleName_ = "SetIsPcAppInPad";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->property_ = windowSessionProperty;
    ASSERT_EQ(WSError::WS_OK, sceneSession->SetIsPcAppInPad(false));
}

/**
 * @tc.name: SetWindowRectAutoSaveCallback
 * @tc.desc: SetWindowRectAutoSaveCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, SetWindowRectAutoSaveCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetWindowRectAutoSaveCallback";
    info.bundleName_ = "SetWindowRectAutoSaveCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->property_ = windowSessionProperty;

    NotifySetWindowRectAutoSaveFunc func1 = [](bool enabled, bool isSaveBySpecifiedFlag) { return; };
    sceneSession->SetWindowRectAutoSaveCallback(std::move(func1));
    ASSERT_NE(nullptr, sceneSession->onSetWindowRectAutoSaveFunc_);
}

/**
 * @tc.name: RegisterSupportWindowModesCallback
 * @tc.desc: RegisterSupportWindowModesCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, RegisterSupportWindowModesCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RegisterSupportWindowModesCallback";
    info.bundleName_ = "RegisterSupportWindowModesCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    NotifySetSupportedWindowModesFunc func1 =
        [sceneSession](std::vector<AppExecFwk::SupportWindowMode>&& supportedWindowModes) { return; };

    sceneSession->RegisterSupportWindowModesCallback(std::move(func1));
    ASSERT_NE(nullptr, sceneSession->onSetSupportedWindowModesFunc_);
}

/**
 * @tc.name: AddExtensionTokenInfo
 * @tc.desc: AddExtensionTokenInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, AddExtensionTokenInfo, TestSize.Level1)
{
    UIExtensionTokenInfo tokenInfo;
    tokenInfo.abilityToken = nullptr;
    SessionInfo info;
    info.abilityName_ = "AddExtensionTokenInfo";
    info.bundleName_ = "AddExtensionTokenInfo";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->AddExtensionTokenInfo(tokenInfo);
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: RemoveExtensionTokenInfo
 * @tc.desc: RemoveExtensionTokenInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, RemoveExtensionTokenInfo, TestSize.Level1)
{
    UIExtensionTokenInfo tokenInfo;
    tokenInfo.abilityToken = nullptr;
    SessionInfo info;
    info.abilityName_ = "RemoveExtensionTokenInfo";
    info.bundleName_ = "RemoveExtensionTokenInfo";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    auto res = sceneSession->extensionTokenInfos_.size();
    sceneSession->RemoveExtensionTokenInfo(tokenInfo.abilityToken);
    EXPECT_EQ(sceneSession->extensionTokenInfos_.size() <= res, true);
}

/**
 * @tc.name: OnNotifyAboveLockScreen
 * @tc.desc: OnNotifyAboveLockScreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, OnNotifyAboveLockScreen, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnNotifyAboveLockScreen";
    info.bundleName_ = "OnNotifyAboveLockScreen";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->OnNotifyAboveLockScreen();
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->CheckExtensionOnLockScreenToClose();
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: CloseExtensionSync
 * @tc.desc: CloseExtensionSync
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, CloseExtensionSync, TestSize.Level1)
{
    UIExtensionTokenInfo tokenInfo;
    tokenInfo.abilityToken = nullptr;
    SessionInfo info;
    info.abilityName_ = "CloseExtensionSync";
    info.bundleName_ = "CloseExtensionSync";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->CloseExtensionSync(tokenInfo);
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: SetIsStatusBarVisible
 * @tc.desc: SetIsStatusBarVisible
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, SetIsStatusBarVisible, TestSize.Level1)
{
    bool isVisible = true;
    SessionInfo info;
    info.abilityName_ = "SetIsStatusBarVisible";
    info.bundleName_ = "SetIsStatusBarVisible";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->SetIsStatusBarVisible(isVisible);
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: GetAllAvoidAreas
 * @tc.desc: GetAllAvoidAreas
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, GetAllAvoidAreas, TestSize.Level1)
{
    std::map<AvoidAreaType, AvoidArea> avoidAreas;
    SessionInfo info;
    info.abilityName_ = "GetAllAvoidAreas";
    info.bundleName_ = "GetAllAvoidAreas";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    auto res = sceneSession->GetAllAvoidAreas(avoidAreas);
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: NotifyPipWindowSizeChange
 * @tc.desc: NotifyPipWindowSizeChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, NotifyPipWindowSizeChange, TestSize.Level1)
{
    uint32_t width = 10;
    uint32_t height = 10;
    double scale = 10;
    SessionInfo info;
    info.abilityName_ = "NotifyPipWindowSizeChange";
    info.bundleName_ = "NotifyPipWindowSizeChange";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    auto res = sceneSession->NotifyPipWindowSizeChange(width, height, scale);
    EXPECT_EQ(res, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: NotifyPiPActiveStatusChange
 * @tc.desc: NotifyPiPActiveStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, NotifyPiPActiveStatusChange, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyPiPActiveStatusChange";
    info.bundleName_ = "NotifyPiPActiveStatusChange";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    auto res = sceneSession->NotifyPiPActiveStatusChange(true);
    EXPECT_EQ(res, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: SendPointEventForMoveDrag
 * @tc.desc: SendPointEventForMoveDrag
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, SendPointEventForMoveDrag, TestSize.Level1)
{
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    bool isExecuteDelayRaise = true;
    SessionInfo info;
    info.abilityName_ = "SendPointEventForMoveDrag";
    info.bundleName_ = "SendPointEventForMoveDrag";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    auto res = sceneSession->SendPointEventForMoveDrag(pointerEvent, isExecuteDelayRaise);
    EXPECT_EQ(res, WSError::WS_OK);
}

/**
 * @tc.name: DisallowActivationFromPendingBackground
 * @tc.desc: DisallowActivationFromPendingBackground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, DisallowActivationFromPendingBackground, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "DisallowActivationFromPendingBackground";
    info.bundleName_ = "DisallowActivationFromPendingBackground";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    auto res = sceneSession->DisallowActivationFromPendingBackground(true, true, true, true);
    EXPECT_EQ(res, false);
}

/**
 * @tc.name: UpdateFullScreenWaterfallMode
 * @tc.desc: UpdateFullScreenWaterfallMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, UpdateFullScreenWaterfallMode, TestSize.Level1)
{
    bool isWaterfallMode = true;
    SessionInfo info;
    info.abilityName_ = "UpdateFullScreenWaterfallMode";
    info.bundleName_ = "UpdateFullScreenWaterfallMode";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->UpdateFullScreenWaterfallMode(isWaterfallMode);
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: OnWaterfallButtonChange
 * @tc.desc: OnWaterfallButtonChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, OnWaterfallButtonChange, TestSize.Level1)
{
    bool isShow = true;
    SessionInfo info;
    info.abilityName_ = "OnWaterfallButtonChange";
    info.bundleName_ = "OnWaterfallButtonChange";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->pcFoldScreenController_ = sptr<PcFoldScreenController>::MakeSptr(wptr(sceneSession),
        sceneSession->GetPersistentId());
    sceneSession->OnWaterfallButtonChange(isShow);
    EXPECT_NE(sceneSession, nullptr);

    sceneSession->pcFoldScreenController_ = nullptr;
    sceneSession->OnWaterfallButtonChange(isShow);
    EXPECT_EQ(sceneSession->pcFoldScreenController_, nullptr);
}

/**
 * @tc.name: RegisterFullScreenWaterfallModeChangeCallback
 * @tc.desc: RegisterFullScreenWaterfallModeChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, RegisterFullScreenWaterfallModeChangeCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RegisterFullScreenWaterfallModeChangeCallback";
    info.bundleName_ = "RegisterFullScreenWaterfallModeChangeCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    std::function<void(bool isWaterfallMode)> func = [](bool isWaterfallMode) { return; };
    sceneSession->RegisterFullScreenWaterfallModeChangeCallback(std::move(func));
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: RegisterThrowSlipAnimationStateChangeCallback
 * @tc.desc: RegisterThrowSlipAnimationStateChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, RegisterThrowSlipAnimationStateChangeCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RegisterThrowSlipAnimationStateChangeCallback";
    info.bundleName_ = "RegisterThrowSlipAnimationStateChangeCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    std::function<void(bool isAnimating, bool isFullScreen)> func = [](bool isAnimating, bool isFullScreen) { return; };
    sceneSession->RegisterThrowSlipAnimationStateChangeCallback(std::move(func));
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: MaskSupportEnterWaterfallMode
 * @tc.desc: MaskSupportEnterWaterfallMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, MaskSupportEnterWaterfallMode, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "MaskSupportEnterWaterfallMode";
    info.bundleName_ = "MaskSupportEnterWaterfallMode";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->MaskSupportEnterWaterfallMode();
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->pcFoldScreenController_ = nullptr;
    sceneSession->MaskSupportEnterWaterfallMode();
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: HandleActionUpdateMainWindowTopmost
 * @tc.desc: HandleActionUpdateMainWindowTopmost
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, HandleActionUpdateMainWindowTopmost, TestSize.Level1)
{
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS;
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateMainWindowTopmost";
    info.bundleName_ = "HandleActionUpdateMainWindowTopmost";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->property_ = windowSessionProperty;

    auto res = sceneSession->HandleActionUpdateMainWindowTopmost(windowSessionProperty, action);
    EXPECT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: HandleActionUpdateSubWindowZLevel
 * @tc.desc: HandleActionUpdateSubWindowZLevel
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, HandleActionUpdateSubWindowZLevel, TestSize.Level1)
{
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_SUB_WINDOW_Z_LEVEL;
    SessionInfo info;
    info.abilityName_ = "HandleActionUpdateSubWindowZLevel";
    info.bundleName_ = "HandleActionUpdateSubWindowZLevel";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->property_ = windowSessionProperty;

    auto res = sceneSession->HandleActionUpdateSubWindowZLevel(windowSessionProperty, action);
    EXPECT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: HandleBackgroundAlpha
 * @tc.desc: HandleBackgroundAlpha
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, HandleBackgroundAlpha, TestSize.Level1)
{
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS;
    SessionInfo info;
    info.abilityName_ = "HandleBackgroundAlpha";
    info.bundleName_ = "HandleBackgroundAlpha";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->property_ = windowSessionProperty;

    auto res = sceneSession->HandleBackgroundAlpha(windowSessionProperty, action);
    EXPECT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: SendContainerModalEvent
 * @tc.desc: SendContainerModalEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, SendContainerModalEvent, TestSize.Level1)
{
    string eventName = "test";
    string eventValue = "test";
    SessionInfo info;
    info.abilityName_ = "SendContainerModalEvent";
    info.bundleName_ = "SendContainerModalEvent";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();

    auto res = sceneSession->SendContainerModalEvent(eventName, eventValue);
    EXPECT_EQ(res, sceneSession->sessionStage_->SendContainerModalEvent(eventName, eventValue));

    eventName = "win_change_water_fall_button";
    eventValue = "win_water_fall_button_show";
    EXPECT_EQ(WSError::WS_OK, sceneSession->SendContainerModalEvent(eventName, eventValue));
}

/**
 * @tc.name: RegisterSetLandscapeMultiWindowFunc
 * @tc.desc: RegisterSetLandscapeMultiWindowFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, RegisterSetLandscapeMultiWindowFunc, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RegisterSetLandscapeMultiWindowFunc";
    info.bundleName_ = "RegisterSetLandscapeMultiWindowFunc";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    std::function<void(bool isLandscapeMultiWindow)> func = [](bool isLandscapeMultiWindow) { return; };
    sceneSession->RegisterSetLandscapeMultiWindowFunc(std::move(func));
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: IsDirtyDragWindow
 * @tc.desc: IsDirtyDragWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, IsDirtyDragWindow, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsDirtyDragWindow";
    info.bundleName_ = "IsDirtyDragWindow";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->dirtyFlags_ = 0;

    auto res = sceneSession->IsDirtyDragWindow();
    EXPECT_EQ(res,
              sceneSession->dirtyFlags_ & static_cast<uint32_t>(SessionUIDirtyFlag::DRAG_RECT) ||
                  sceneSession->isDragging_);
}

/**
 * @tc.name: RequestHideKeyboard
 * @tc.desc: RequestHideKeyboard
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, RequestHideKeyboard, TestSize.Level1)
{
    bool isAppColdStart = true;
    SessionInfo info;
    info.abilityName_ = "RequestHideKeyboard";
    info.bundleName_ = "RequestHideKeyboard";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->RequestHideKeyboard(isAppColdStart);
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: SetStartingWindowExitAnimationFlag
 * @tc.desc: SetStartingWindowExitAnimationFlag
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, SetStartingWindowExitAnimationFlag, TestSize.Level1)
{
    bool enable = true;
    SessionInfo info;
    info.abilityName_ = "SetStartingWindowExitAnimationFlag";
    info.bundleName_ = "SetStartingWindowExitAnimationFlag";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    sceneSession->SetStartingWindowExitAnimationFlag(enable);
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: SetSkipSelfWhenShowOnVirtualScreen01
 * @tc.desc: SetSkipSelfWhenShowOnVirtualScreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, SetSkipSelfWhenShowOnVirtualScreen01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetSkipSelfWhenShowOnVirtualScreen";
    info.bundleName_ = "SetSkipSelfWhenShowOnVirtualScreen";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    bool skipResult = false;

    sceneSession->SetSkipSelfWhenShowOnVirtualScreen(false);
    usleep(SLEEP_TIME_US);
    ASSERT_EQ(skipResult, false);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    surfaceNode->id_ = 1;
    sceneSession->surfaceNode_ = surfaceNode;

    sceneSession->SetSkipSelfWhenShowOnVirtualScreen(false);
    usleep(SLEEP_TIME_US);
    ASSERT_EQ(skipResult, false);
}

/**
 * @tc.name: RegisterRequestedOrientationChangeCallback
 * @tc.desc: RegisterRequestedOrientationChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, RegisterRequestedOrientationChangeCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RegisterRequestedOrientationChangeCallback";
    info.bundleName_ = "RegisterRequestedOrientationChangeCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    std::function<void(uint32_t orientation, bool needAnimation)> func = [](uint32_t orientation, bool needAnimation) {
        return;
    };
    sceneSession->RegisterRequestedOrientationChangeCallback(std::move(func));
    EXPECT_NE(sceneSession, nullptr);
}

/**
 * @tc.name: IsShowOnLockScreen
 * @tc.desc: IsShowOnLockScreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, IsShowOnLockScreen, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsShowOnLockScreen";
    info.bundleName_ = "IsShowOnLockScreen";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetZOrder(2005);
    sceneSession->GetSessionProperty()->SetDisplayId(-2);
    EXPECT_FALSE(sceneSession->IsShowOnLockScreen(2000));
    sceneSession->GetSessionProperty()->SetDisplayId(-1);
    EXPECT_TRUE(sceneSession->IsShowOnLockScreen(2000));

    sceneSession->SetZOrder(0);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_FALSE(sceneSession->IsShowOnLockScreen(2000));

    sptr<SceneSession> parentSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    parentSession->SetZOrder(2004);
    sceneSession->parentSession_ = parentSession;
    EXPECT_TRUE(sceneSession->IsShowOnLockScreen(2000));

    sceneSession->SetZOrder(1000);
    EXPECT_FALSE(sceneSession->IsShowOnLockScreen(2000));

    sceneSession->property_ = nullptr;
    EXPECT_FALSE(sceneSession->IsShowOnLockScreen(2000));
}

/**
 * @tc.name: NotifySetParentSession
 * @tc.desc: NotifySetParentSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, NotifySetParentSession, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifySetParentSession";
    info.bundleName_ = "NotifySetParentSession";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    int32_t oldParentWindowId = 1;
    int32_t newParentWindowId = 2;
    auto res = sceneSession->NotifySetParentSession(oldParentWindowId, newParentWindowId);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
}

/**
 * @tc.name: UpdateSubWindowLevel
 * @tc.desc: UpdateSubWindowLevel
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, UpdateSubWindowLevel, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateSubWindowLevel";
    info.bundleName_ = "UpdateSubWindowLevel";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    uint32_t subWindowLevel = 10;
    EXPECT_NE(subWindowLevel, sceneSession->GetSessionProperty()->GetSubWindowLevel());
    sceneSession->UpdateSubWindowLevel(subWindowLevel);
    EXPECT_EQ(subWindowLevel, sceneSession->GetSessionProperty()->GetSubWindowLevel());
}

/**
 * @tc.name: UpdateLifecyclePausedInner
 * @tc.desc: UpdateLifecyclePausedInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, UpdateLifecyclePausedInner, TestSize.Level1)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    SessionInfo info;
    info.abilityName_ = "UpdateLifecyclePausedInner";
    info.bundleName_ = "UpdateLifecyclePausedInner";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session, nullptr);
    session->UpdateLifecyclePausedInner();
    EXPECT_TRUE(logMsg.find("state: ") == std::string::npos);
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    session->sessionStage_ = mockSessionStage;
    session->state_ = SessionState::STATE_ACTIVE;
    session->UpdateLifecyclePausedInner();
    session->state_ = SessionState::STATE_FOREGROUND;
    session->UpdateLifecyclePausedInner();
    session->state_ = SessionState::STATE_BACKGROUND;
    session->UpdateLifecyclePausedInner();
    EXPECT_TRUE(logMsg.find("state: ") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetUseControlState
 * @tc.desc: SetUseControlState and GetUseControlState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, SetUseControlState, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    bool isUseControl = true;
    property->SetUseControlState(isUseControl);
    EXPECT_EQ(true, property->GetUseControlState());
}

/**
 * @tc.name: OnSessionEvent2
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, OnSessionEvent2, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "OnSessionEvent2";
    info.bundleName_ = "OnSessionEvent2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    sceneSession->moveDragController_ = sptr<MoveDragController>::MakeSptr(wptr(sceneSession));
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowMode(WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    sceneSession->property_ = property;
    auto ret = sceneSession->OnSessionEvent(SessionEvent::EVENT_START_MOVE);
    usleep(SLEEP_TIME_US);
    EXPECT_EQ(ret, WSError::WS_OK);

    property->SetWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    ret = sceneSession->OnSessionEvent(SessionEvent::EVENT_START_MOVE);
    usleep(SLEEP_TIME_US);
    EXPECT_EQ(ret, WSError::WS_OK);

    property->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    ret = sceneSession->OnSessionEvent(SessionEvent::EVENT_START_MOVE);
    usleep(SLEEP_TIME_US);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: RegisterUpdateAppUseControlCallback
 * @tc.desc: RegisterUpdateAppUseControlCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, RegisterUpdateAppUseControlCallback, Function | SmallTest | Level3)
{
    ControlInfo controlInfo = {
        .isNeedControl = false,
        .isControlRecentOnly = true
    };
    ControlInfo controlInfoSec = {
        .isNeedControl = false,
        .isControlRecentOnly = false
    };
    ControlInfo controlInfoThd = {
        .isNeedControl = true,
        .isControlRecentOnly = false
    };
    SessionInfo info;
    info.bundleName_ = "app";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto callback = [](ControlAppType type, bool isNeedControl, bool isControlRecentOnly) {};
 
    std::unordered_map<std::string, std::unordered_map<ControlAppType, ControlInfo>> allAppUseMap;
    sceneSession->SetGetAllAppUseControlMapFunc([&allAppUseMap]() ->
        std::unordered_map<std::string, std::unordered_map<ControlAppType, ControlInfo>>& {return allAppUseMap;});
    sceneSession->RegisterUpdateAppUseControlCallback(callback);
    std::string key = "app#0";
    allAppUseMap[key][ControlAppType::APP_LOCK] = controlInfo;
    sceneSession->RegisterUpdateAppUseControlCallback(callback);

    std::unordered_map<std::string, std::unordered_map<ControlAppType, ControlInfo>> allAppUseMapSec;
    sceneSession->SetGetAllAppUseControlMapFunc([&allAppUseMapSec]() ->
        std::unordered_map<std::string, std::unordered_map<ControlAppType, ControlInfo>>& {return allAppUseMapSec;});
    sceneSession->RegisterUpdateAppUseControlCallback(callback);
    std::string keySec = "app#0";
    allAppUseMapSec[keySec][ControlAppType::APP_LOCK] = controlInfoSec;
    sceneSession->RegisterUpdateAppUseControlCallback(callback);

    std::unordered_map<std::string, std::unordered_map<ControlAppType, ControlInfo>> allAppUseMapThd;
    sceneSession->SetGetAllAppUseControlMapFunc([&allAppUseMapThd]() ->
        std::unordered_map<std::string, std::unordered_map<ControlAppType, ControlInfo>>& {return allAppUseMapThd;});
    sceneSession->RegisterUpdateAppUseControlCallback(callback);
    std::string keyThd = "app#0";
    allAppUseMapThd[keyThd][ControlAppType::APP_LOCK] = controlInfoThd;
    sceneSession->RegisterUpdateAppUseControlCallback(callback);
    ASSERT_NE(nullptr, sceneSession->onUpdateAppUseControlFunc_);
}

/**
 * @tc.name: NotifyUpdateAppUseControl01
 * @tc.desc: NotifyUpdateAppUseControl
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, NotifyUpdateAppUseControl01, Function | SmallTest | Level3)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ControlInfo controlInfo = {
        .isNeedControl = true,
        .isControlRecentOnly = false
    };
    SessionInfo info;
    info.abilityName_ = "NotifyUpdateAppUseControl";
    info.bundleName_ = "NotifyUpdateAppUseControl";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto callback = [](ControlAppType type, bool isNeedControl, bool isControlRecentOnly) {
        std::cout << "isNeedControl:" << isNeedControl << ";isControlRecentOnly:" << isControlRecentOnly << std::endl;
    };
    sceneSession->onUpdateAppUseControlFunc_ = std::move(callback);
    sceneSession->sessionStage_ = nullptr;
    ControlAppType type = ControlAppType::APP_LOCK;
    sceneSession->NotifyUpdateAppUseControl(type, controlInfo);
    sceneSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    sceneSession->NotifyUpdateAppUseControl(type, controlInfo);

    ControlInfo controlInfoSec = {
        .isNeedControl = false,
        .isControlRecentOnly = false
    };
    sceneSession->NotifyUpdateAppUseControl(type, controlInfoSec);

    sceneSession->Session::SetSessionState(SessionState::STATE_BACKGROUND);
    sceneSession->NotifyUpdateAppUseControl(type, controlInfoSec);
    ControlInfo controlInfoThd = {
        .isNeedControl = true,
        .isControlRecentOnly = false
    };
    sceneSession->NotifyUpdateAppUseControl(type, controlInfoThd);
    EXPECT_TRUE(logMsg.find("isAppUseControl:") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: NotifyUpdateAppUseControl02
 * @tc.desc: NotifyUpdateAppUseControl
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, NotifyUpdateAppUseControl02, Function | SmallTest | Level3)
{
    logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    SessionInfo info;
    info.abilityName_ = "NotifyUpdateAppUseControl";
    info.bundleName_ = "NotifyUpdateAppUseControl";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto callback = [](ControlAppType type, bool isNeedControl, bool isControlRecentOnly) {
        std::cout << "isNeedControl:" << isNeedControl << ";isControlRecentOnly:" << isControlRecentOnly << std::endl;
    };
    sceneSession->onUpdateAppUseControlFunc_ = std::move(callback);
    sceneSession->sessionStage_ = nullptr;
    ControlAppType type = ControlAppType::APP_LOCK;
    sceneSession->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();

    ControlInfo controlInfo1 = {
        .isNeedControl = false,
        .isControlRecentOnly = true
    };
    sceneSession->NotifyUpdateAppUseControl(type, controlInfo1);

    sceneSession->Session::SetSessionState(SessionState::STATE_BACKGROUND);
    sceneSession->NotifyUpdateAppUseControl(type, controlInfo1);
    ControlInfo controlInfo2 = {
        .isNeedControl = true,
        .isControlRecentOnly = true
    };
    sceneSession->NotifyUpdateAppUseControl(type, controlInfo2);
    EXPECT_TRUE(logMsg.find("isAppUseControl:") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: GetMaxSubWindowLevel
 * @tc.desc: GetMaxSubWindowLevel
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, GetMaxSubWindowLevel, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetMaxSubWindowLevel";
    info.bundleName_ = "GetMaxSubWindowLevel";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_EQ(1, sceneSession->GetMaxSubWindowLevel());
}

/**
 * @tc.name: NotifyUpdateFlagCallback
 * @tc.desc: NotifyUpdateFlagCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, NotifyUpdateFlagCallback, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyUpdateFlagCallback";
    info.bundleName_ = "NotifyUpdateFlagCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);

    NotifyUpdateFlagFunc func1 = [](const std::string& flag) { return; };
    sceneSession->NotifyUpdateFlagCallback(std::move(func1));
    ASSERT_NE(nullptr, sceneSession->onUpdateFlagFunc_);
}

/**
 * @tc.name: GetKeyboardOccupiedAreaWithRotation1
 * @tc.desc: GetKeyboardOccupiedAreaWithRotation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, GetKeyboardOccupiedAreaWithRotation1, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetKeyboardOccupiedAreaWithRotation";
    info.bundleName_ = "GetKeyboardOccupiedAreaWithRotation";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->specificCallback_ = nullptr;

    int32_t persistentId = 1;
    std::vector<std::pair<bool, WSRect>> avoidAreas;
    std::pair<bool, WSRect> keyboardOccupiedArea = { false, { 0, 0, 0, 0 } };
    avoidAreas.emplace_back(keyboardOccupiedArea);
    sceneSession->GetKeyboardOccupiedAreaWithRotation(persistentId, Rotation::ROTATION_90, avoidAreas);
    uint32_t areaSize = static_cast<uint32_t>(avoidAreas.size());
    ASSERT_EQ(1, areaSize);
    ASSERT_EQ(false, avoidAreas[0].first);
}

/**
 * @tc.name: GetKeyboardOccupiedAreaWithRotation2
 * @tc.desc: GetKeyboardOccupiedAreaWithRotation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, GetKeyboardOccupiedAreaWithRotation2, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetKeyboardOccupiedAreaWithRotation";
    info.bundleName_ = "GetKeyboardOccupiedAreaWithRotation";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sceneSession->specificCallback_ = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sceneSession->specificCallback_->onKeyboardRotationChange_ = nullptr;

    int32_t persistentId = 1;
    std::vector<std::pair<bool, WSRect>> avoidAreas;
    std::pair<bool, WSRect> keyboardOccupiedArea = { true, { 0, 0, 0, 0 } };
    avoidAreas.emplace_back(keyboardOccupiedArea);
    sceneSession->GetKeyboardOccupiedAreaWithRotation(persistentId, Rotation::ROTATION_90, avoidAreas);
    uint32_t areaSize = static_cast<uint32_t>(avoidAreas.size());
    ASSERT_EQ(1, areaSize);
    ASSERT_EQ(true, avoidAreas[0].first);
}

/**
 * @tc.name: SetSkipEventOnCastPlus01
 * @tc.desc: check func SetSkipEventOnCastPlus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionTest3, SetSkipEventOnCastPlus01, TestSize.Level1)
{
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    SessionInfo info;
    info.abilityName_ = "SetSkipEventOnCastPlus";
    info.bundleName_ = "SetSkipEventOnCastPlus";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->SetSkipEventOnCastPlus(false);
    ASSERT_EQ(false, sceneSession->GetSessionProperty()->GetSkipEventOnCastPlus());
}
} // namespace
} // namespace Rosen
} // namespace OHOS