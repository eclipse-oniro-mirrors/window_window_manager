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

#include <algorithm>

#include <gtest/gtest.h>
#include <pointer_event.h>

#include "application_context.h"
#include "common/include/session_permission.h"
#include "display_manager.h"
#include "input_event.h"
#include "key_event.h"
#include "mock/mock_accesstoken_kit.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_scene_session.h"
#include "mock_vsync_station.h"
#include "pointer_event.h"
#include "proxy/include/window_info.h"
#include "screen_manager.h"

#include "session/host/include/main_session.h"
#include "session/host/include/keyboard_session.h"
#define PRIVATE public
#define PROTECTED public
#include "session/host/include/scene_session.h"
#undef PRIVATE
#undef PROTECTED
#include "session/host/include/sub_session.h"
#include "session/host/include/system_session.h"
#include "session/screen/include/screen_session.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "transaction/rs_transaction.h"
#include "ui/rs_surface_node.h"
#include "window_helper.h"
#include "wm_common.h"
#include "dm_common.h"
#include "parameters.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace Rosen {
namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }
}
class SceneSessionRotationTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void SceneSessionRotationTest::SetUpTestCase() {}

void SceneSessionRotationTest::TearDownTestCase() {}

void SceneSessionRotationTest::SetUp() {}

void SceneSessionRotationTest::TearDown() {}

namespace {
/**
 * @tc.name: SetDefaultRequestedOrientation
 * @tc.desc: SetDefaultRequestedOrientation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, SetDefaultRequestedOrientation, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetDefaultRequestedOrientation";
    info.bundleName_ = "SetDefaultRequestedOrientation";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    Orientation orientation = Orientation::AUTO_ROTATION_UNSPECIFIED;
    sceneSession->SetDefaultRequestedOrientation(orientation);
    Orientation ret = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(orientation, ret);

    orientation = Orientation::USER_ROTATION_PORTRAIT;
    sceneSession->SetDefaultRequestedOrientation(orientation);
    ret = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(orientation, ret);

    orientation = Orientation::USER_ROTATION_LANDSCAPE;
    sceneSession->SetDefaultRequestedOrientation(orientation);
    ret = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(orientation, ret);

    orientation = Orientation::USER_ROTATION_PORTRAIT_INVERTED;
    sceneSession->SetDefaultRequestedOrientation(orientation);
    ret = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(orientation, ret);

    orientation = Orientation::USER_ROTATION_LANDSCAPE_INVERTED;
    sceneSession->SetDefaultRequestedOrientation(orientation);
    ret = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(orientation, ret);

    orientation = Orientation::FOLLOW_DESKTOP;
    sceneSession->SetDefaultRequestedOrientation(orientation);
    ret = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(orientation, ret);
}

/**
 * @tc.name: SetRequestedOrientation
 * @tc.desc: SetRequestedOrientation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, SetRequestedOrientation01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetRequestedOrientation01";
    info.bundleName_ = "SetRequestedOrientation01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    Orientation ori = Orientation::UNSPECIFIED;
    sceneSession->SetRequestedOrientation(ori);
    Orientation ret = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ori, ret);

    sceneSession->SetRequestedOrientation(Orientation::AUTO_ROTATION_UNSPECIFIED);
    Orientation ret1 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret1, Orientation::AUTO_ROTATION_UNSPECIFIED);

    sceneSession->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT);
    Orientation ret2 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret2, Orientation::USER_ROTATION_PORTRAIT);

    sceneSession->SetRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE);
    Orientation ret3 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret3, Orientation::USER_ROTATION_LANDSCAPE);

    sceneSession->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT_INVERTED);
    Orientation ret4 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret4, Orientation::USER_ROTATION_PORTRAIT_INVERTED);

    sceneSession->SetRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE_INVERTED);
    Orientation ret5 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret5, Orientation::USER_ROTATION_LANDSCAPE_INVERTED);

    sceneSession->SetRequestedOrientation(Orientation::FOLLOW_DESKTOP);
    Orientation ret6 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret6, Orientation::FOLLOW_DESKTOP);
}

/**
 * @tc.name: GetRequestedOrientation
 * @tc.desc: GetRequestedOrientation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, GetRequestedOrientation, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetRequestedOrientation";
    info.bundleName_ = "GetRequestedOrientation";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    Orientation ori = Orientation::HORIZONTAL;
    sceneSession->SetRequestedOrientation(ori);
    Orientation ret = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ori, ret);

    sceneSession->SetRequestedOrientation(Orientation::AUTO_ROTATION_UNSPECIFIED);
    Orientation ret1 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret1, Orientation::AUTO_ROTATION_UNSPECIFIED);

    sceneSession->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT);
    Orientation ret2 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret2, Orientation::USER_ROTATION_PORTRAIT);

    sceneSession->SetRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE);
    Orientation ret3 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret3, Orientation::USER_ROTATION_LANDSCAPE);

    sceneSession->SetRequestedOrientation(Orientation::USER_ROTATION_PORTRAIT_INVERTED);
    Orientation ret4 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret4, Orientation::USER_ROTATION_PORTRAIT_INVERTED);

    sceneSession->SetRequestedOrientation(Orientation::USER_ROTATION_LANDSCAPE_INVERTED);
    Orientation ret5 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret5, Orientation::USER_ROTATION_LANDSCAPE_INVERTED);

    sceneSession->SetRequestedOrientation(Orientation::FOLLOW_DESKTOP);
    Orientation ret6 = sceneSession->GetRequestedOrientation();
    ASSERT_EQ(ret6, Orientation::FOLLOW_DESKTOP);
}

/**
 * @tc.name: NotifyRotationChange
 * @tc.desc: NotifyRotationChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, NotifyRotationChange, Function | SmallTest | Level2)
{
    const SessionInfo info;
    sptr<SceneSessionMocker> session = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    session->sessionStage_ = nullptr;
    session->isRotationChangeCallbackRegistered = false;
    RotationChangeInfo rotationInfo = { RotationChangeType::WINDOW_WILL_ROTATE, 0, 0, { 0, 0, 2720, 1270 } };
    RotationChangeResult res = session->NotifyRotationChange(rotationInfo);
    EXPECT_EQ(res.windowRect_.width_, 0);

    session->isRotationChangeCallbackRegistered = true;
    res = session->NotifyRotationChange(rotationInfo);
    EXPECT_EQ(res.windowRect_.width_, 0);

    sptr<SessionStageMocker> sessionStageMocker = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(sessionStageMocker, nullptr);
    session->sessionStage_ = sessionStageMocker;
    res = session->NotifyRotationChange(rotationInfo);
    EXPECT_EQ(res.windowRect_.width_, 0);
}

/**
 * @tc.name: NotifyRotationChange_IsRestrictNotify_SystemWindow
 * @tc.desc: NotifyRotationChange_IsRestrictNotify_SystemWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, NotifyRotationChange_IsRestrictNotify_SystemWindow, Function | SmallTest | Level2)
{
    sptr<SessionStageMocker> sessionStageMocker = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(sessionStageMocker, nullptr);
    RotationChangeInfo rotationInfo = { RotationChangeType::WINDOW_WILL_ROTATE, 0, 0, { 0, 0, 2720, 1270 } };
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    SessionInfo info;
    property->SetWindowType(WindowType::WINDOW_TYPE_DESKTOP);
    sptr<SceneSessionMocker> session = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    session->isRotationChangeCallbackRegistered = true;
    session->sessionStage_ = sessionStageMocker;
    bool isRestrictNotify = false;
    property->isSystemCalling_ = true;
    session->SetSessionProperty(property);
    RotationChangeResult res = session->NotifyRotationChange(rotationInfo, isRestrictNotify);
    EXPECT_EQ(res.windowRect_.width_, 0);

    property->isSystemCalling_ = false;
    session->SetSessionProperty(property);
    res = session->NotifyRotationChange(rotationInfo, isRestrictNotify);
    EXPECT_EQ(res.windowRect_.width_, 0);

    isRestrictNotify = true;
    property->isSystemCalling_ = true;
    session->SetSessionProperty(property);
    res = session->NotifyRotationChange(rotationInfo, isRestrictNotify);
    EXPECT_EQ(res.windowRect_.width_, 0);

    property->isSystemCalling_ = false;
    session->SetSessionProperty(property);
    res = session->NotifyRotationChange(rotationInfo, isRestrictNotify);
    EXPECT_EQ(res.windowRect_.width_, 0);
}

/**
 * @tc.name: NotifyRotationChange_IsRestrictNotify_NotSystemWindow
 * @tc.desc: NotifyRotationChange_IsRestrictNotify_NotSystemWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, NotifyRotationChange_IsRestrictNotify_NotSystemWindow, Function | SmallTest | Level2)
{
    sptr<SessionStageMocker> sessionStageMocker = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(sessionStageMocker, nullptr);
    RotationChangeInfo rotationInfo = { RotationChangeType::WINDOW_WILL_ROTATE, 0, 0, { 0, 0, 2720, 1270 } };
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    SessionInfo info;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSessionMocker> session = sptr<SceneSessionMocker>::MakeSptr(info, nullptr);
    session->isRotationChangeCallbackRegistered = true;
    session->sessionStage_ = sessionStageMocker;
    bool isRestrictNotify = false;
    property->isSystemCalling_ = true;
    session->SetSessionProperty(property);
    RotationChangeResult res = session->NotifyRotationChange(rotationInfo, isRestrictNotify);
    EXPECT_EQ(res.windowRect_.width_, 0);

    property->isSystemCalling_ = false;
    session->SetSessionProperty(property);
    res = session->NotifyRotationChange(rotationInfo, isRestrictNotify);
    EXPECT_EQ(res.windowRect_.width_, 0);

    isRestrictNotify = true;
    property->isSystemCalling_ = true;
    session->SetSessionProperty(property);
    res = session->NotifyRotationChange(rotationInfo, isRestrictNotify);
    EXPECT_EQ(res.windowRect_.width_, 0);

    property->isSystemCalling_ = false;
    session->SetSessionProperty(property);
    res = session->NotifyRotationChange(rotationInfo, isRestrictNotify);
    EXPECT_EQ(res.windowRect_.width_, 0);
}

/**
 * @tc.name: ConvertRotationToOrientation
 * @tc.desc: ConvertRotationToOrientation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, ConvertRotationToOrientation, Function | SmallTest | Level2)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    SessionInfo info;
    info.abilityName_ = "ConvertRotationToOrientation";
    info.bundleName_ = "ConvertRotationToOrientation";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->GetSessionProperty()->SetDisplayId(1001);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1001, screenSession);

    uint32_t orientation = 0;
    WSError result = sceneSession->ConvertRotationToOrientation(90, 1, 1, orientation);
    EXPECT_EQ(result, WSError::WS_OK);

    sceneSession->GetSessionProperty()->SetDisplayId(1024);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1024, nullptr);
    uint32_t orientation1 = 0;
    WSError result1 = sceneSession->ConvertRotationToOrientation(90, 1, 1, orientation1);
    EXPECT_EQ(result1, WSError::WS_ERROR_INVALID_DISPLAY);
    EXPECT_TRUE(g_errLog.find("Screen session is null") != std::string::npos);
}

/**
 * @tc.name: GetSystemBarPropertyForRotation
 * @tc.desc: GetSystemBarPropertyForRotation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, GetSystemBarPropertyForRotation, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "GetSystemBarPropertyForRotation";
    info.bundleName_ = "GetSystemBarPropertyForRotation";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);

    std::map<Rosen::WindowType, Rosen::SystemBarProperty> properties;
    sceneSession->SetSystemBarPropertyForRotation(properties);
    EXPECT_EQ(sceneSession->GetSystemBarPropertyForRotation(), properties);
}

/**
 * @tc.name: RegisterRotationLockChangeCallback
 * @tc.desc: RegisterRotationLockChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, RegisterRotationLockChangeCallback, TestSize.Level0)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    sceneSession->specificCallback_ = nullptr;
    auto task = [] (bool locked) {};
    sceneSession->RegisterRotationLockChangeCallback(std::move(task));
    EXPECT_EQ(nullptr, sceneSession->specificCallback_);

    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, callback);
    sceneSession->specificCallback_ = callback;
    sceneSession->RegisterRotationLockChangeCallback(nullptr);
    EXPECT_EQ(nullptr, callback->onRotationLockChange_);
    sceneSession->RegisterRotationLockChangeCallback(std::move(task));
    EXPECT_NE(nullptr, callback->onRotationLockChange_);
}

/**
 * @tc.name: HandleActionUpdateRotationLockChange
 * @tc.desc: HandleActionUpdateRotationLockChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, HandleActionUpdateRotationLockChange, TestSize.Level0)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetRotationLocked(false);
    sceneSession->specificCallback_ = nullptr;
    WSPropertyChangeAction action = WSPropertyChangeAction::ACTION_UPDATE_ROTATION_LOCK_CHANGE;
    WMError ret = sceneSession->HandleActionUpdateRotationLockChange(property, action);
    EXPECT_EQ(ret, WMError::WM_OK);

    sptr<SceneSession::SpecificSessionCallback> callback = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(nullptr, callback);
    sceneSession->specificCallback_ = callback;
    ret = sceneSession->HandleActionUpdateRotationLockChange(property, action);
    EXPECT_EQ(ret, WMError::WM_OK);

    auto task = [] (bool locked) {};
    callback->onRotationLockChange_ = task;
    ret = sceneSession->HandleActionUpdateRotationLockChange(property, action);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: NotifyPageRotationIsIgnored
 * @tc.desc: NotifyPageRotationIsIgnored function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, NotifyPageRotationIsIgnored, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "NotifyPageRotationIsIgnored";
    info.bundleName_ = "NotifyPageRotationIsIgnored";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, session);
    auto ret = session->NotifyPageRotationIsIgnored();
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: ConvertOrientationAndRotation
 * @tc.desc: ConvertOrientationAndRotation function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, ConvertOrientationAndRotation, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ConvertOrientationAndRotation";
    info.bundleName_ = "ConvertOrientationAndRotation";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, session);
    session->GetSessionProperty()->SetDisplayId(1001);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1001, screenSession);

    RotationInfoType from = RotationInfoType::DISPLAY_ORIENTATION;
    RotationInfoType to = RotationInfoType::DISPLAY_ORIENTATION;
    int32_t value = -1;
    int32_t convertedValue = 0;
    auto ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);

    value = 4;
    ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);

    value = 2;
    from = RotationInfoType::DISPLAY_ROTATION;
    to = RotationInfoType::DISPLAY_ROTATION;
    ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);

    from = RotationInfoType::DISPLAY_ROTATION;
    to = RotationInfoType::WINDOW_ORIENTATION;
    ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);

    from = RotationInfoType::DISPLAY_ROTATION;
    to = RotationInfoType::DISPLAY_ORIENTATION;
    ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);

    from = RotationInfoType::DISPLAY_ORIENTATION;
    to = RotationInfoType::WINDOW_ORIENTATION;
    ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);

    from = RotationInfoType::DISPLAY_ORIENTATION;
    to = RotationInfoType::DISPLAY_ROTATION;
    ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);

    from = RotationInfoType::WINDOW_ORIENTATION;
    to = RotationInfoType::DISPLAY_ORIENTATION;
    ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);

    from = RotationInfoType::WINDOW_ORIENTATION;
    to = RotationInfoType::DISPLAY_ROTATION;
    ret = session->ConvertOrientationAndRotation(from, to, value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: ConvertDisplayOrientationToWindowOrientation
 * @tc.desc: ConvertDisplayOrientationToWindowOrientation function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, ConvertDisplayOrientationToWindowOrientation, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ConvertDisplayOrientationToWindowOrientation";
    info.bundleName_ = "ConvertDisplayOrientationToWindowOrientation";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, session);
    int32_t value = 4;
    int32_t convertedValue = 0;

    auto ret = session->ConvertDisplayOrientationToWindowOrientation(value, convertedValue);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);
    value = 5;
    ret = session->ConvertDisplayOrientationToWindowOrientation(value, convertedValue);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);
    value = 2;
    ret = session->ConvertDisplayOrientationToWindowOrientation(value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: ConvertWindowOrientationToDisplayOrientation
 * @tc.desc: ConvertWindowOrientationToDisplayOrientation function
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, ConvertWindowOrientationToDisplayOrientation, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ConvertWindowOrientationToDisplayOrientation";
    info.bundleName_ = "ConvertWindowOrientationToDisplayOrientation";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, session);
    int32_t value = 4;
    int32_t convertedValue = 0;
    value = 5;
    auto ret = session->ConvertWindowOrientationToDisplayOrientation(value, convertedValue);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, ret);
    value = 2;
    ret = session->ConvertWindowOrientationToDisplayOrientation(value, convertedValue);
    EXPECT_EQ(WSError::WS_OK, ret);
}

/**
 * @tc.name: ConvertDisplayRotationToDisplayOrientation
 * @tc.desc: ConvertDisplayRotationToDisplayOrientation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, ConvertDisplayRotationToDisplayOrientation, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "ConvertDisplayRotationToDisplayOrientation";
    info.bundleName_ = "ConvertDisplayRotationToDisplayOrientation";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->GetSessionProperty()->SetDisplayId(1001);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1001, screenSession);

    int32_t value = 2;
    int32_t convertedValue = 0;
    WSError result = sceneSession->ConvertDisplayRotationToDisplayOrientation(value, convertedValue);
    EXPECT_EQ(result, WSError::WS_OK);

    sceneSession->GetSessionProperty()->SetDisplayId(1024);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1024, nullptr);
    WSError result1 = sceneSession->ConvertDisplayRotationToDisplayOrientation(value, convertedValue);
    EXPECT_EQ(result1, WSError::WS_ERROR_INVALID_DISPLAY);
}

/**
 * @tc.name: ConvertDisplayOrientationToDisplayRotation
 * @tc.desc: ConvertDisplayOrientationToDisplayRotation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, ConvertDisplayOrientationToDisplayRotation, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "ConvertDisplayOrientationToDisplayRotation";
    info.bundleName_ = "ConvertDisplayOrientationToDisplayRotation";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->GetSessionProperty()->SetDisplayId(1001);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1001, screenSession);

    int32_t value = 2;
    int32_t convertedValue = 0;
    WSError result = sceneSession->ConvertDisplayOrientationToDisplayRotation(value, convertedValue);
    EXPECT_EQ(result, WSError::WS_OK);

    sceneSession->GetSessionProperty()->SetDisplayId(1024);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1024, nullptr);
    WSError result1 = sceneSession->ConvertDisplayOrientationToDisplayRotation(value, convertedValue);
    EXPECT_EQ(result1, WSError::WS_ERROR_INVALID_DISPLAY);
}

/**
 * @tc.name: ConvertDisplayRotationToWindowOrientation
 * @tc.desc: ConvertDisplayRotationToWindowOrientation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, ConvertDisplayRotationToWindowOrientation, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "ConvertDisplayRotationToWindowOrientation";
    info.bundleName_ = "ConvertDisplayRotationToWindowOrientation";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->GetSessionProperty()->SetDisplayId(1001);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1001, screenSession);

    int32_t value = 2;
    int32_t convertedValue = 0;
    WSError result = sceneSession->ConvertDisplayRotationToWindowOrientation(value, convertedValue);
    EXPECT_EQ(result, WSError::WS_OK);

    sceneSession->GetSessionProperty()->SetDisplayId(1024);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1024, nullptr);
    WSError result1 = sceneSession->ConvertDisplayRotationToWindowOrientation(value, convertedValue);
    EXPECT_EQ(result1, WSError::WS_ERROR_INVALID_DISPLAY);
}

/**
 * @tc.name: ConvertWindowOrientationToDisplayRotation
 * @tc.desc: ConvertWindowOrientationToDisplayRotation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionRotationTest, ConvertWindowOrientationToDisplayRotation, Function | SmallTest | Level2)
{
    SessionInfo info;
    info.abilityName_ = "ConvertWindowOrientationToDisplayRotation";
    info.bundleName_ = "ConvertWindowOrientationToDisplayRotation";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->GetSessionProperty()->SetDisplayId(1001);
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr(config,
        ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1001, screenSession);

    int32_t value = 2;
    int32_t convertedValue = 0;
    WSError result = sceneSession->ConvertWindowOrientationToDisplayRotation(value, convertedValue);
    EXPECT_EQ(result, WSError::WS_OK);

    sceneSession->GetSessionProperty()->SetDisplayId(1024);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(1024, nullptr);
    WSError result1 = sceneSession->ConvertWindowOrientationToDisplayRotation(value, convertedValue);
    EXPECT_EQ(result1, WSError::WS_ERROR_INVALID_DISPLAY);
}
} // namespace
} // namespace Rosen
} // namespace OHOS