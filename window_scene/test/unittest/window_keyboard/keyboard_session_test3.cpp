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

#include "display_manager.h"
#include "session/host/include/keyboard_session.h"
#include <gtest/gtest.h>
#include <ui/rs_surface_node.h>

#include "interfaces/include/ws_common.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_keyboard_session.h"
#include "session/host/include/session.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session/host/include/scene_session.h"

#define protected public
#include "session/host/include/session.h"
#undef protected

#define private public
#include "session/host/include/keyboard_session.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t SLEEP_TIME_US = 100000; // 100ms
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
        const char* msg)
    {
        g_logMsg += msg;
    }
}
class KeyboardSessionTest3 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<KeyboardSession> GetKeyboardSession(const std::string& abilityName, const std::string& bundleName);
    sptr<SceneSession> GetSceneSession(const std::string& abilityName, const std::string& bundleName);
private:
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
};

void KeyboardSessionTest3::SetUpTestCase() {}

void KeyboardSessionTest3::TearDownTestCase() {}

void KeyboardSessionTest3::SetUp() {}

void KeyboardSessionTest3::TearDown() {}

sptr<KeyboardSession> KeyboardSessionTest3::GetKeyboardSession(const std::string& abilityName,
                                                               const std::string& bundleName)
{
    SessionInfo info;
    info.abilityName_ = abilityName;
    info.bundleName_ = bundleName;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    EXPECT_NE(specificCb, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCb =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    EXPECT_NE(keyboardCb, nullptr);
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCb, keyboardCb);
    EXPECT_NE(keyboardSession, nullptr);

    sptr<WindowSessionProperty> keyboardProperty = sptr<WindowSessionProperty>::MakeSptr();
    EXPECT_NE(keyboardProperty, nullptr);
    keyboardProperty->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    keyboardSession->SetSessionProperty(keyboardProperty);

    SessionInfo info1;
    info1.abilityName_ = "BindKeyboardPanelSession";
    info1.bundleName_ = "BindKeyboardPanelSession";
    sptr<SceneSession> panelSession = sptr<SceneSession>::MakeSptr(info1, nullptr);
    keyboardSession->BindKeyboardPanelSession(panelSession);

    if (!handler_) {
        auto runner = AppExecFwk::EventRunner::Create("KeyboardSessionTest3");
        handler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    keyboardSession->SetEventHandler(handler_, nullptr);

    return keyboardSession;
}

sptr<SceneSession> KeyboardSessionTest3::GetSceneSession(const std::string& abilityName, const std::string& bundleName)
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
 * @tc.name: IsVisibleNotBackground
 * @tc.desc: test IsVisibleNotBackground
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, IsVisibleNotBackground, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsVisibleNotBackground";
    info.bundleName_ = "IsVisibleNotBackground";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, nullptr, nullptr);
    keyboardSession->isVisible_ = false;
    EXPECT_EQ(false, keyboardSession->IsVisibleNotBackground());
    keyboardSession->isVisible_ = true;
    EXPECT_EQ(true, keyboardSession->IsVisibleNotBackground());
}

/**
 * @tc.name: GetRSTransaction01
 * @tc.desc: test function: GetRSTransaction
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, GetRSTransaction01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetRSTransaction01", "GetRSTransaction01");
    ASSERT_NE(keyboardSession, nullptr);

    auto rsTransaction = keyboardSession->GetRSTransaction();
    ASSERT_EQ(rsTransaction, nullptr);
}

/**
 * @tc.name: GetSessionScreenName01
 * @tc.desc: test function: GetSessionScreenName
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, GetSessionScreenName01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("GetSessionScreenName01", "GetSessionScreenName01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->property_ = nullptr;
    auto resultStr = keyboardSession->GetSessionScreenName();
    ASSERT_EQ(resultStr, "");

    sptr<WindowSessionProperty> keyboardProperty = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(keyboardProperty, nullptr);
    keyboardSession->property_ = keyboardProperty;
    keyboardSession->property_->displayId_ = 100;

    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ASSERT_NE(screenSession, nullptr);
    screenSession->name_ = "testScreenSession";
    screenSession->screenId_ = 100;
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.emplace(100, screenSession);

    resultStr = keyboardSession->GetSessionScreenName();
    ASSERT_EQ(resultStr, screenSession->name_);
}

/**
 * @tc.name: UseFocusIdIfCallingSessionIdInvalid001
 * @tc.desc: test function: UseFocusIdIfCallingSessionIdInvalid
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, UseFocusIdIfCallingSessionIdInvalid001, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UseFocusIdIfCallingSessionIdInvalid001",
        "UseFocusIdIfCallingSessionIdInvalid001");
    ASSERT_NE(keyboardSession, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCallback =
        new (std::nothrow) KeyboardSession::KeyboardSessionCallback();
    ASSERT_NE(keyboardCallback, nullptr);
    keyboardSession->keyboardCallback_ = keyboardCallback;
    sptr<SceneSession> sceneSession = GetSceneSession("TestSceneSession", "TestSceneSession");
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->persistentId_ = 100;
    keyboardSession->keyboardCallback_->onGetSceneSession =
        [sceneSession](uint32_t callingSessionId)->sptr<SceneSession> {
            if (sceneSession->persistentId_ != callingSessionId) {
                return nullptr;
            }
            return sceneSession;
        };
    
    // calling session is invalid, calling id == focus id
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() -> int32_t { return 100; };
    keyboardSession->GetSessionProperty()->SetCallingSessionId(1234);
    sceneSession->GetSessionProperty()->SetDisplayId(11111);
    keyboardSession->GetSessionProperty()->SetDisplayId(22222);
    keyboardSession->UseFocusIdIfCallingSessionIdInvalid(100); // use calling session
    auto resultId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(resultId, 100);

    // calling session is valid, display id matches
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() -> int32_t { return 777; };
    keyboardSession->GetSessionProperty()->SetCallingSessionId(1234);
    sceneSession->GetSessionProperty()->SetDisplayId(33333);
    keyboardSession->GetSessionProperty()->SetDisplayId(33333);
    keyboardSession->UseFocusIdIfCallingSessionIdInvalid(100); // use calling session
    resultId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(resultId, 100); // use calling session

    // calling session is valid, neither condition is true
    sceneSession->persistentId_ = 999;
    keyboardSession->GetSessionProperty()->SetCallingSessionId(1234); // initial value, to be replaced
    constexpr auto myFocusedSessionId = 4444;
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() -> int32_t { return myFocusedSessionId; };
    keyboardSession->keyboardCallback_->onGetSceneSession =
        [sceneSession](uint32_t callingSessionId) -> sptr<SceneSession> {
            if (callingSessionId == 999 || callingSessionId == myFocusedSessionId) {
                return sceneSession;
            }
            return nullptr;
        };
    sceneSession->GetSessionProperty()->SetDisplayId(11111); // display id mismatches
    keyboardSession->GetSessionProperty()->SetDisplayId(22222);
    keyboardSession->UseFocusIdIfCallingSessionIdInvalid(999); // calling id != focused id
    resultId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(resultId, 4444); // replaced with focused id
}

/**
 * @tc.name: UseFocusIdIfCallingSessionIdInvalid002
 * @tc.desc: test function: UseFocusIdIfCallingSessionIdInvalid
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, UseFocusIdIfCallingSessionIdInvalid002, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UseFocusIdIfCallingSessionIdInvalid002",
        "UseFocusIdIfCallingSessionIdInvalid002");
    ASSERT_NE(keyboardSession, nullptr);
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCallback =
        new (std::nothrow) KeyboardSession::KeyboardSessionCallback();
    ASSERT_NE(keyboardCallback, nullptr);
    keyboardSession->keyboardCallback_ = keyboardCallback;
    sptr<SceneSession> sceneSession = GetSceneSession("TestSceneSession", "TestSceneSession");
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->persistentId_ = 100;
    keyboardSession->keyboardCallback_->onGetSceneSession =
        [sceneSession](uint32_t callingSessionId) -> sptr<SceneSession> {
            if (sceneSession->persistentId_ == callingSessionId) {
                return sceneSession;
            }
            return nullptr;
        };
    
    // calling session is invalid, use focused id
    keyboardSession->GetSessionProperty()->SetCallingSessionId(1234); // initial value, to be replaced by 100
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() -> int32_t { return 100; };
    keyboardSession->keyboardCallback_->onGetSceneSession =
        [sceneSession](uint32_t callingSessionId) -> sptr<SceneSession> {
            if (callingSessionId == 100) {
                return sceneSession; // return something for 100
            }
            return nullptr;
        };
    keyboardSession->UseFocusIdIfCallingSessionIdInvalid(123456); // invalid calling session id, use focused id
    auto resultId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(resultId, 100);

    keyboardSession->keyboardCallback_->onCallingSessionIdChange = [](uint32_t) {};
    // calling session is invalid, use focusesd id with callback
    keyboardSession->GetSessionProperty()->SetCallingSessionId(1234); // initial value, to be replaced by 100
    keyboardSession->keyboardCallback_->onGetFocusedSessionId = []() -> int32_t { return 100; };
    keyboardSession->keyboardCallback_->onGetSceneSession =
        [sceneSession](uint32_t callingSessionId) -> sptr<SceneSession> {
            if (callingSessionId == 100) {
                return sceneSession; // return something for 100
            }
            return nullptr;
        };
    keyboardSession->UseFocusIdIfCallingSessionIdInvalid(123456); // invalid calling session id, use focused id
    resultId = keyboardSession->GetCallingSessionId();
    EXPECT_EQ(resultId, 100);
}

/**
 * @tc.name: SetSurfaceBounds01
 * @tc.desc: test function: SetSurfaceBounds
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, SetSurfaceBounds01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("SetSurfaceBounds01", "SetSurfaceBounds01");
    ASSERT_NE(keyboardSession, nullptr);
    WSRect rect = { 0, 0, 0, 0 };

    keyboardSession->surfaceNode_ = nullptr;
    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_BOTTOM;
    keyboardSession->SetSurfaceBounds(rect, false);
    ASSERT_EQ(keyboardSession->GetKeyboardGravity(), SessionGravity::SESSION_GRAVITY_BOTTOM);

    keyboardSession->property_->keyboardLayoutParams_.gravity_ = WindowGravity::WINDOW_GRAVITY_FLOAT;
    keyboardSession->keyboardPanelSession_ = nullptr;
    keyboardSession->SetSurfaceBounds(rect, false);
    ASSERT_EQ(keyboardSession->GetKeyboardGravity(), SessionGravity::SESSION_GRAVITY_FLOAT);

    SessionInfo info1;
    info1.abilityName_ = "BindKeyboardPanelSession";
    info1.bundleName_ = "BindKeyboardPanelSession";
    sptr<SceneSession> panelSession = sptr<SceneSession>::MakeSptr(info1, nullptr);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    keyboardSession->SetSurfaceBounds(rect, false);
    ASSERT_NE(keyboardSession->GetKeyboardPanelSession(), nullptr);

    struct RSSurfaceNodeConfig config;
    keyboardSession->surfaceNode_ = RSSurfaceNode::Create(config);
    keyboardSession->SetSurfaceBounds(rect, false);
    ASSERT_NE(keyboardSession->surfaceNode_, nullptr);
}

/**
 * @tc.name: HandleKeyboardMoveDragEnd01
 * @tc.desc: test function: HandleKeyboardMoveDragEnd
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, HandleKeyboardMoveDragEnd01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("HandleKeyboardMoveDragEnd01", "HandleKeyboardMoveDragEnd01");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->adjustKeyboardLayoutFunc_ = [](const KeyboardLayoutParams& params) {};

    WSRect rect = { 50, 50, 900, 900 };
    keyboardSession->HandleKeyboardMoveDragEnd(rect, SizeChangeReason::DRAG_END, -1);
    keyboardSession->HandleKeyboardMoveDragEnd(rect, SizeChangeReason::DRAG_END, 11);
    keyboardSession->sessionRectChangeFunc_ = [](const WSRect& rect,
                                                 SizeChangeReason reason,
                                                 DisplayId displayId) { return; };
    keyboardSession->HandleKeyboardMoveDragEnd(rect, SizeChangeReason::DRAG_END, -1);
    keyboardSession->HandleKeyboardMoveDragEnd(rect, SizeChangeReason::DRAG_END, 11);
}

/**
 * @tc.name: UpdateSizeChangeReason01
 * @tc.desc: test function: UpdateSizeChangeReason
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, UpdateSizeChangeReason01, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("UpdateSizeChangeReason01", "UpdateSizeChangeReason01");
    ASSERT_NE(keyboardSession, nullptr);
    auto keyboardPanelSession = keyboardSession->GetKeyboardPanelSession();
    ASSERT_NE(keyboardPanelSession, nullptr);
    ASSERT_EQ(WSError::WS_OK, keyboardSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_END));
    usleep(SLEEP_TIME_US);
    ASSERT_EQ(keyboardSession->GetSizeChangeReason(), SizeChangeReason::DRAG_END);
    ASSERT_EQ(keyboardPanelSession->GetSizeChangeReason(), SizeChangeReason::DRAG_END);
}

/**
 * @tc.name: RecalculatePanelRectForAvoidArea
 * @tc.desc: test function: RecalculatePanelRectForAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, RecalculatePanelRectForAvoidArea, TestSize.Level1)
{
    auto keyboardSession = GetKeyboardSession("RecalculatePanelRectForAvoidArea", "RecalculatePanelRectForAvoidArea");

    // if landscapeAvoidHeight_ or portraitAvoidHeight_ < 0
    WSRect panelRect = { 0, 0, 0, 0 };
    KeyboardLayoutParams params;
    keyboardSession->GetSessionProperty()->SetKeyboardLayoutParams(params);
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
    EXPECT_EQ(panelRect.height_, 0);
    params.landscapeAvoidHeight_ = 1;
    params.portraitAvoidHeight_ = -1;
    keyboardSession->GetSessionProperty()->SetKeyboardLayoutParams(params);
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
    EXPECT_EQ(panelRect.height_, 0);
    params.landscapeAvoidHeight_ = -1;
    params.portraitAvoidHeight_ = 1;
    keyboardSession->GetSessionProperty()->SetKeyboardLayoutParams(params);
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
    EXPECT_EQ(panelRect.height_, 0);
    params.landscapeAvoidHeight_ = 1;
    params.portraitAvoidHeight_ = 1;
    // the landscape width is same to the portrait
    keyboardSession->GetSessionProperty()->SetKeyboardLayoutParams(params);
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
    EXPECT_EQ(panelRect.height_, 1);
}

/**
 * @tc.name: RecalculatePanelRectForAvoidArea02
 * @tc.desc: test function: RecalculatePanelRectForAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, RecalculatePanelRectForAvoidArea02, TestSize.Level1)
{
    auto keyboardSession =
        GetKeyboardSession("RecalculatePanelRectForAvoidArea02", "RecalculatePanelRectForAvoidArea02");

    // if the landscape width is not same to the portrait
    KeyboardLayoutParams params;
    params.landscapeAvoidHeight_ = 1;
    params.portraitAvoidHeight_ = 2;
    params.LandscapePanelRect_.width_ = 1;
    params.PortraitPanelRect_.width_ = 2;
    keyboardSession->GetSessionProperty()->SetKeyboardLayoutParams(params);

    WSRect panelRect = { 0, 0, 0, 0 };
    panelRect.width_ = 1;
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
    EXPECT_EQ(panelRect.height_, 1);
    panelRect.width_ = 2;
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
    EXPECT_EQ(panelRect.height_, 2);
    panelRect.width_ = 3;
    params.portraitAvoidHeight_ = 1;
    keyboardSession->GetSessionProperty()->SetKeyboardLayoutParams(params);
    keyboardSession->RecalculatePanelRectForAvoidArea(panelRect);
    EXPECT_EQ(panelRect.height_, 1);
}

/**
 * @tc.name: SetSkipSelfWhenShowOnVirtualScreen
 * @tc.desc: test function: SetSkipSelfWhenShowOnVirtualScreen
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, SetSkipSelfWhenShowOnVirtualScreen, TestSize.Level1)
{
    auto keyboardSession =
        GetKeyboardSession("SetSkipSelfWhenShowOnVirtualScreen", "SetSkipSelfWhenShowOnVirtualScreen");
    bool skipResult = false;
    auto callFunc = [&skipResult](uint64_t surfaceNodeId, bool isSkip) { skipResult = isSkip; };
    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(true);
    usleep(SLEEP_TIME_US);
    ASSERT_EQ(skipResult, false);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    surfaceNode->id_ = 1;
    keyboardSession->surfaceNode_ = surfaceNode;

    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(true);
    usleep(SLEEP_TIME_US);
    ASSERT_EQ(skipResult, false);

    keyboardSession->specificCallback_->onSetSkipSelfWhenShowOnVirtualScreen_ = callFunc;
    keyboardSession->SetSkipSelfWhenShowOnVirtualScreen(true);
    usleep(SLEEP_TIME_US);
    ASSERT_EQ(skipResult, true);
}

/**
 * @tc.name: SetSkipEventOnCastPlus01
 * @tc.desc: check func SetSkipEventOnCastPlus
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, SetSkipEventOnCastPlus01, Function | SmallTest | Level0)
{
    auto keyboardSession = GetKeyboardSession(" SetSkipEventOnCastPlus01", " SetSkipEventOnCastPlus01");

    keyboardSession->SetSkipEventOnCastPlus(false);
    ASSERT_EQ(false, keyboardSession->GetSessionProperty()->GetSkipEventOnCastPlus());
}

/**
 * @tc.name: IsNeedRaiseSubWindow01
 * @tc.desc: check func IsNeedRaiseSubWindow
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, IsNeedRaiseSubWindow01, Function | SmallTest | Level0)
{
    auto keyboardSession = GetKeyboardSession("IsNeedRaiseSubWindow", "IsNeedRaiseSubWindow");
    SessionInfo info;
    info.abilityName_ = "CallingSession";
    info.bundleName_ = "CallingSession";
    auto callingSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto sessionRect = callingSession->GetSessionRect();
    auto ret = keyboardSession->IsNeedRaiseSubWindow(callingSession, sessionRect);
    ASSERT_EQ(true, ret);

    callingSession->GetSessionProperty()->type_ = WindowType::WINDOW_TYPE_APP_SUB_WINDOW;
    ret = keyboardSession->IsNeedRaiseSubWindow(callingSession, sessionRect);
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: IsNeedRaiseSubWindow02
 * @tc.desc: check func IsNeedRaiseSubWindow
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, IsNeedRaiseSubWindow02, Function | SmallTest | Level0)
{
    auto keyboardSession = GetKeyboardSession("IsNeedRaiseSubWindow", "IsNeedRaiseSubWindow");
    SessionInfo info;
    info.abilityName_ = "CallingSession";
    info.bundleName_ = "CallingSession";
    auto callingSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto sessionRect = callingSession->GetSessionRect();
    callingSession->GetSessionProperty()->type_ = WindowType::WINDOW_TYPE_APP_SUB_WINDOW;

    SessionInfo mainInfo;
    mainInfo.abilityName_ = "mainSession";
    mainInfo.bundleName_ = "mainSession";
    auto mainSession = sptr<SceneSession>::MakeSptr(mainInfo, nullptr);
    callingSession->parentSession_ = mainSession;

    auto ret = keyboardSession->IsNeedRaiseSubWindow(callingSession, sessionRect);
    ASSERT_EQ(true, ret);

    mainSession->GetSessionProperty()->windowMode_ = WindowMode::WINDOW_MODE_SPLIT_PRIMARY;
    ret = keyboardSession->IsNeedRaiseSubWindow(callingSession, sessionRect);
    ASSERT_EQ(false, ret);

    WSRect newRect{ 0, 0, 0, 10 };
    mainSession->SetSessionRect(newRect);
    ret = keyboardSession->IsNeedRaiseSubWindow(callingSession, sessionRect);
    ASSERT_EQ(true, ret);
}

/**
 * @tc.name: CalculateOccupiedArea_MIDSCENE
 * @tc.desc: check func CalculateOccupiedArea_MIDSCENE
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, CalculateOccupiedArea_MIDSCENE, Function | SmallTest | Level1)
{
    auto keyboardSession = GetKeyboardSession("CalculateOccupiedArea_MIDSCENE",
        "CalculateOccupiedArea_MIDSCENE");
    SessionInfo info;
    info.abilityName_ = "CallingSession";
    info.bundleName_ = "CallingSession";
    auto callingSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    WSRect lastSafeRect = { 0, 0, 0, 0 };
    callingSession->SetLastSafeRect(lastSafeRect);
    WSRect zeroRect1 = { 0, 0, 0, 0 };
    WSRect zeroRect2 = { 0, 0, 0, 0 };
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo = nullptr;

    auto ret = keyboardSession->CalculateOccupiedArea(nullptr, zeroRect1, zeroRect2, occupiedAreaInfo);
    EXPECT_EQ(ret, false);

    ret = keyboardSession->CalculateOccupiedArea(callingSession, zeroRect1, zeroRect2, occupiedAreaInfo);
    EXPECT_EQ(ret, false);

    lastSafeRect = { 1, 2, 3, 4 };
    callingSession->SetLastSafeRect(lastSafeRect);
    ret = keyboardSession->CalculateOccupiedArea(callingSession, zeroRect1, zeroRect2, occupiedAreaInfo);
    EXPECT_EQ(ret, true);

    callingSession->SetIsMidScene(true);
    usleep(SLEEP_TIME_US);
    EXPECT_EQ(true, callingSession->GetIsMidScene());
    lastSafeRect = { 1, 2, 3, 4 };
    callingSession->SetLastSafeRect(lastSafeRect);
    ret = keyboardSession->CalculateOccupiedArea(callingSession, zeroRect1, zeroRect2, occupiedAreaInfo);
    EXPECT_EQ(ret, true);

    callingSession->SetIsMidScene(true);
    usleep(SLEEP_TIME_US);
    EXPECT_EQ(true, callingSession->GetIsMidScene());
    callingSession->SetScale(0.0, 0.0, 0.0, 0.0);
    lastSafeRect = { 0, 0, 0, 0 };
    callingSession->SetLastSafeRect(lastSafeRect);
    zeroRect1 = { 0, 0, 1, 2 };
    zeroRect2 = { 0, 0, 1, 2 };
    ret = keyboardSession->CalculateOccupiedArea(callingSession, zeroRect1, zeroRect2, occupiedAreaInfo);
    EXPECT_EQ(ret, false);

    callingSession->SetIsMidScene(true);
    usleep(SLEEP_TIME_US);
    EXPECT_EQ(true, callingSession->GetIsMidScene());
    callingSession->SetScale(0.0, 0.0, 0.0, 0.0);
    lastSafeRect = { 1, 2, 3, 4 };
    callingSession->SetLastSafeRect(lastSafeRect);
    zeroRect1 = { 0, 0, 1, 2 };
    zeroRect2 = { 0, 0, 1, 2 };
    ret = keyboardSession->CalculateOccupiedArea(callingSession, zeroRect1, zeroRect2, occupiedAreaInfo);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: CalculateOccupiedAreaAfterUIRefresh01
 * @tc.desc: check func CalculateOccupiedAreaAfterUIRefresh
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, CalculateOccupiedAreaAfterUIRefresh01, Function | SmallTest | Level1)
{
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCallback =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    keyboardCallback->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo callingSessionInfo;
        callingSessionInfo.abilityName_ = "CallingSession";
        callingSessionInfo.bundleName_ = "CallingSession";
        auto callingSession = sptr<SceneSession>::MakeSptr(callingSessionInfo, nullptr);
        callingSession->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
        callingSession->persistentId_ = persistentId;
        return callingSession;
    };

    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();

    SessionInfo info;
    info.abilityName_ = "keyboardSession";
    info.bundleName_ = "keyboardSession";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCallback, keyboardCallback);
    keyboardSession->property_->SetCallingSessionId(36);
    keyboardSession->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::VISIBLE);
    keyboardSession->isVisible_ = true;

    keyboardSession->CalculateOccupiedAreaAfterUIRefresh();
    EXPECT_EQ(keyboardSession->stateChanged_, false);

    keyboardSession->isVisible_ = false;
    keyboardSession->dirtyFlags_ &= ~static_cast<uint32_t>(SessionUIDirtyFlag::VISIBLE);
    keyboardSession->CalculateOccupiedAreaAfterUIRefresh();
    EXPECT_EQ(keyboardSession->stateChanged_, false);

    keyboardCallback->onGetSceneSession = [](uint32_t persistentId) { return nullptr; };
    keyboardSession->isVisible_ = true;
    keyboardSession->CalculateOccupiedAreaAfterUIRefresh();
    EXPECT_EQ(keyboardSession->stateChanged_, false);

    keyboardSession->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
    keyboardCallback->onGetSceneSession = [](uint32_t persistentId) {
        SessionInfo callingSessionInfo;
        callingSessionInfo.abilityName_ = "CallingSession";
        callingSessionInfo.bundleName_ = "CallingSession";
        auto callingSession = sptr<SceneSession>::MakeSptr(callingSessionInfo, nullptr);
        callingSession->persistentId_ = persistentId;
        return callingSession;
    };
    keyboardSession->CalculateOccupiedAreaAfterUIRefresh();
    EXPECT_EQ(keyboardSession->stateChanged_, false);

    keyboardSession->stateChanged_ = true;
    keyboardSession->dirtyFlags_ &= ~static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
    keyboardSession->CalculateOccupiedAreaAfterUIRefresh();
    EXPECT_EQ(keyboardSession->stateChanged_, false);
}

/**
 * @tc.name: CalculateOccupiedAreaAfterUIRefresh02
 * @tc.desc: check func CalculateOccupiedAreaAfterUIRefresh
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, CalculateOccupiedAreaAfterUIRefresh02, Function | SmallTest | Level1)
{
    sptr<KeyboardSession::KeyboardSessionCallback> keyboardCallback =
        sptr<KeyboardSession::KeyboardSessionCallback>::MakeSptr();
    
    SessionInfo callingSessionInfo;
    callingSessionInfo.abilityName_ = "CallingSession";
    callingSessionInfo.bundleName_ = "CallingSession";
    auto callingSession = sptr<SceneSession>::MakeSptr(callingSessionInfo, nullptr);
    callingSession->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
    keyboardCallback->onGetSceneSession = [&](uint32_t persistentId) {
        callingSession->persistentId_ = persistentId;
        return callingSession;
    };
    sptr<SceneSession::SpecificSessionCallback> specificCallback =
        sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    
    SessionInfo info;
    info.abilityName_ = "keyboardSession";
    info.bundleName_ = "keyboardSession";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(info, specificCallback, keyboardCallback);
    keyboardSession->property_->SetCallingSessionId(36);
    keyboardSession->isVisible_ = true;

    callingSession->UpdateSizeChangeReason(SizeChangeReason::DRAG);
    callingSession->SetOriPosYBeforeRaisedByKeyboard(100);
    keyboardSession->CalculateOccupiedAreaAfterUIRefresh();
    EXPECT_EQ(callingSession->GetOriPosYBeforeRaisedByKeyboard(), 0);

    callingSession->UpdateSizeChangeReason(SizeChangeReason::DRAG_MOVE);
    keyboardSession->CalculateOccupiedAreaAfterUIRefresh();
    EXPECT_EQ(callingSession->GetOriPosYBeforeRaisedByKeyboard(), 0);

    callingSession->SetOriPosYBeforeRaisedByKeyboard(200);
    callingSession->UpdateSizeChangeReason(SizeChangeReason::ROTATION);
    EXPECT_EQ(keyboardSession->stateChanged_, false);
    EXPECT_EQ(callingSession->GetOriPosYBeforeRaisedByKeyboard(), 200);
}

/**
 * @tc.name: SetKeyboardEffectOptionChangeListener
 * @tc.desc: check func SetKeyboardEffectOptionChangeListener
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, SetKeyboardEffectOptionChangeListener, Function | SmallTest | Level0)
{
    auto keyboardSession = GetKeyboardSession("GetPanelRect", "GetPanelRect");
    ASSERT_NE(keyboardSession, nullptr);
    keyboardSession->changeKeyboardEffectOptionFunc_ = nullptr;
    keyboardSession->SetKeyboardEffectOptionChangeListener([](const KeyboardEffectOption& effectOption) {});
    usleep(SLEEP_TIME_US);
    ASSERT_NE(keyboardSession->changeKeyboardEffectOptionFunc_, nullptr);
}

/**
 * @tc.name: GetPanelRect
 * @tc.desc: check func GetPanelRect
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, GetPanelRect, Function | SmallTest | Level0)
{
    auto keyboardSession = GetKeyboardSession("GetPanelRect", "GetPanelRect");
    keyboardSession->keyboardPanelSession_ = nullptr;

    WSRect rect = { 0, 0, 0, 0 };
    WSRect panelRect = keyboardSession->GetPanelRect();
    ASSERT_EQ(true, rect == panelRect);

    SessionInfo info1;
    info1.abilityName_ = "BindKeyboardPanelSession";
    info1.bundleName_ = "BindKeyboardPanelSession";
    sptr<SceneSession> panelSession = sptr<SceneSession>::MakeSptr(info1, nullptr);
    keyboardSession->BindKeyboardPanelSession(panelSession);
    ASSERT_NE(keyboardSession->keyboardPanelSession_, nullptr);
    rect = { 1, 2, 3, 4 };
    panelSession->SetSessionRect(rect);
    panelRect = keyboardSession->GetPanelRect();
    ASSERT_EQ(true, rect == panelRect);
}


/**
 * @tc.name: PostKeyboardAnimationSyncTimeoutTask
 * @tc.desc: check func PostKeyboardAnimationSyncTimeoutTask
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, PostKeyboardAnimationSyncTimeoutTask, Function | SmallTest | Level0)
{
    auto keyboardSession = GetKeyboardSession(
        "PostKeyboardAnimationSyncTimeoutTask", "PostKeyboardAnimationSyncTimeoutTask");
    keyboardSession->PostKeyboardAnimationSyncTimeoutTask();
    EXPECT_EQ(false, keyboardSession->isKeyboardSyncTransactionOpen_);

    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->PostKeyboardAnimationSyncTimeoutTask();
    EXPECT_NE(false, keyboardSession->isKeyboardSyncTransactionOpen_);
}

/**
 * @tc.name: CloseRSTransaction
 * @tc.desc: check func CloseRSTransaction
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, CloseRSTransaction, Function | SmallTest | Level0)
{
    auto keyboardSession = GetKeyboardSession("CloseRSTransaction", "CloseRSTransaction");
    keyboardSession->isKeyboardSyncTransactionOpen_ = false;
    keyboardSession->CloseRSTransaction();
    ASSERT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, false);

    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->CloseRSTransaction();
    ASSERT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, false);
}

/**
 * @tc.name: ProcessKeyboardOccupiedAreaInfo
 * @tc.desc: check func ProcessKeyboardOccupiedAreaInfo
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, ProcessKeyboardOccupiedAreaInfo, Function | SmallTest | Level0)
{
    auto keyboardSession = GetKeyboardSession("ProcessKeyboardOccupiedAreaInfo", "ProcessKeyboardOccupiedAreaInfo");
    ASSERT_NE(keyboardSession, nullptr);
    sptr<SceneSession> sceneSession = GetSceneSession("TestSceneSession", "TestSceneSession");
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->persistentId_ = 0;

    bool needRecalculateAvoidAreas = true;
    bool needCheckRSTransaction = true;
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->ProcessKeyboardOccupiedAreaInfo(0, needRecalculateAvoidAreas, needCheckRSTransaction);
    ASSERT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, false);

    keyboardSession->keyboardCallback_->onGetSceneSession =
        [sceneSession](uint32_t callingSessionId) -> sptr<SceneSession> {
        if (sceneSession->persistentId_ != callingSessionId) {
            return nullptr;
        }
        return sceneSession;
    };

    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    keyboardSession->ProcessKeyboardOccupiedAreaInfo(0, needRecalculateAvoidAreas, needCheckRSTransaction);
    ASSERT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, false);

    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    needCheckRSTransaction = false;
    keyboardSession->ProcessKeyboardOccupiedAreaInfo(0, needRecalculateAvoidAreas, needCheckRSTransaction);

    keyboardSession->keyboardCallback_->onGetSceneSession = [&](uint32_t persistentId) {
        return sceneSession;
    };
    keyboardSession->isKeyboardSyncTransactionOpen_ = true;
    needCheckRSTransaction = true;
    keyboardSession->ProcessKeyboardOccupiedAreaInfo(0, needRecalculateAvoidAreas, needCheckRSTransaction);
    ASSERT_EQ(keyboardSession->isKeyboardSyncTransactionOpen_, false);
}

/**
 * @tc.name: NotifyOccupiedAreaChanged
 * @tc.desc: check func NotifyOccupiedAreaChanged
 * @tc.type: FUNC
 */
HWTEST_F(KeyboardSessionTest3, NotifyOccupiedAreaChanged, Function | SmallTest | Level0)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    auto keyboardSession = GetKeyboardSession("NotifyOccupiedAreaChanged", "NotifyOccupiedAreaChanged");
    ASSERT_NE(keyboardSession, nullptr);
    sptr<SceneSession> callingSession = GetSceneSession("TestSceneSession", "TestSceneSession");
    ASSERT_NE(callingSession, nullptr);
    sptr<OccupiedAreaChangeInfo> occupiedAreaInfo = nullptr;

    g_logMsg.clear();
    keyboardSession->NotifyOccupiedAreaChanged(callingSession, occupiedAreaInfo, false, nullptr);
    EXPECT_TRUE(g_logMsg.find("occupiedAreaInfo is null") != std::string::npos);

    g_logMsg.clear();
    callingSession->sessionInfo_.isSystem_ = true;
    occupiedAreaInfo = sptr<OccupiedAreaChangeInfo>::MakeSptr();
    ASSERT_NE(occupiedAreaInfo, nullptr);
    keyboardSession->NotifyOccupiedAreaChanged(callingSession, occupiedAreaInfo, false, nullptr);
    EXPECT_TRUE(g_logMsg.find("Calling id:") != std::string::npos);

    g_logMsg.clear();
    callingSession->sessionInfo_.isSystem_ = false;
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    callingSession->sessionStage_ = mockSessionStage;
    EXPECT_CALL(*mockSessionStage, NotifyOccupiedAreaChangeInfo(_, _, _, _)).Times(2);
    keyboardSession->NotifyOccupiedAreaChanged(callingSession, occupiedAreaInfo, false, nullptr);
    EXPECT_TRUE(g_logMsg.find("Calling id:") != std::string::npos);

    g_logMsg.clear();
    callingSession->sessionInfo_.isSystem_ = false;
    keyboardSession->NotifyOccupiedAreaChanged(callingSession, occupiedAreaInfo, true, nullptr);
    EXPECT_TRUE(g_logMsg.find("size of avoidAreas: 5") != std::string::npos);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
