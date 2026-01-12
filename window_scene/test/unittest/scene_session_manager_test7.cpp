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
#include "iremote_object_mocker.h"
#include "mock/mock_accesstoken_kit.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "session_manager.h"
#define private public
#include "session/host/include/keyboard_session.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const std::string EMPTY_DEVICE_ID = "";
using ConfigItem = WindowSceneConfig::ConfigItem;
} // namespace
class SceneSessionManagerTest7 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static bool gestureNavigationEnabled_;
    static ProcessGestureNavigationEnabledChangeFunc callbackFunc_;
    static sptr<SceneSessionManager> ssm_;

    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
    static constexpr uint32_t WAIT_SYNC_FOR_SNAPSHOT_SKIP_IN_NS = 500000;
    static constexpr uint32_t WAIT_SYNC_FOR_TEST_END_IN_NS = 1000000;
};

sptr<SceneSessionManager> SceneSessionManagerTest7::ssm_ = nullptr;

bool SceneSessionManagerTest7::gestureNavigationEnabled_ = true;
ProcessGestureNavigationEnabledChangeFunc SceneSessionManagerTest7::callbackFunc_ =
    [](bool enable, const std::string& bundleName, GestureBackType type) { gestureNavigationEnabled_ = enable; };

void SceneSessionManagerTest7::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerTest7::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerTest7::SetUp()
{
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerTest7::TearDown()
{
    MockAccesstokenKit::ChangeMockStateToInit();
    usleep(WAIT_SYNC_IN_NS);
    ssm_->sceneSessionMap_.clear();
}

namespace {
/**
 * @tc.name: UpdateSessionWindowVisibilityListener
 * @tc.desc: UpdateSessionWindowVisibilityListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, UpdateSessionWindowVisibilityListener, TestSize.Level1)
{
    int32_t persistentId = 1;
    bool haveListener = true;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    auto ret = ssm_->UpdateSessionWindowVisibilityListener(persistentId, haveListener);
    EXPECT_EQ(ret, WSError::WS_DO_NOTHING);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "UpdateSessionWindowVisibilityListener";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->callingPid_ = 65535;
    ssm_->sceneSessionMap_.insert(std::make_pair(persistentId, sceneSession));
    ret = ssm_->UpdateSessionWindowVisibilityListener(persistentId, haveListener);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: ProcessVirtualPixelRatioChange
 * @tc.desc: ProcessVirtualPixelRatioChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, ProcessVirtualPixelRatioChange, TestSize.Level1)
{
    DisplayId defaultDisplayId = 0;
    sptr<DisplayInfo> displayInfo = nullptr;
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap;
    DisplayStateChangeType type = DisplayStateChangeType::BEFORE_SUSPEND;
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessVirtualPixelRatioChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    displayInfo = sptr<DisplayInfo>::MakeSptr();
    ASSERT_NE(nullptr, displayInfo);
    ssm_->sceneSessionMap_.clear();
    ssm_->ProcessVirtualPixelRatioChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest6";
    sessionInfo.abilityName_ = "UpdateAvoidArea";
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->ProcessVirtualPixelRatioChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    sessionInfo.isSystem_ = false;
    ssm_->ProcessVirtualPixelRatioChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    ssm_->ProcessVirtualPixelRatioChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    ssm_->ProcessVirtualPixelRatioChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    sceneSession->SetSessionState(SessionState::STATE_INACTIVE);
    ssm_->ProcessVirtualPixelRatioChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    sceneSession = nullptr;
    ssm_->ProcessVirtualPixelRatioChange(defaultDisplayId, displayInfo, displayInfoMap, type);
}

/**
 * @tc.name: ProcessVirtualPixelRatioChange01
 * @tc.desc: ProcessVirtualPixelRatioChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, ProcessVirtualPixelRatioChange01, TestSize.Level1)
{
    DisplayId defaultDisplayId = 0;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap;
    DisplayStateChangeType type = DisplayStateChangeType::BEFORE_SUSPEND;
    ASSERT_NE(nullptr, displayInfo);
    ASSERT_NE(nullptr, ssm_);
    ssm_->processVirtualPixelRatioChangeFunc_ = nullptr;
    displayInfo->SetVirtualPixelRatio(0.1f);
    displayInfo->SetDensityInCurResolution(0.1f);
    ssm_->ProcessVirtualPixelRatioChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    ProcessVirtualPixelRatioChangeFunc func = [](float ratio, const OHOS::Rosen::Rect& rect) {};
    ssm_->SetVirtualPixelRatioChangeListener(func);
    ASSERT_NE(nullptr, ssm_->processVirtualPixelRatioChangeFunc_);
    ssm_->ProcessVirtualPixelRatioChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    displayInfo->SetDensityInCurResolution(0.2f);
    ssm_->ProcessVirtualPixelRatioChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    ssm_->processVirtualPixelRatioChangeFunc_ = nullptr;
    ssm_->ProcessVirtualPixelRatioChange(defaultDisplayId, displayInfo, displayInfoMap, type);
}

/**
 * @tc.name: ProcessUpdateRotationChange
 * @tc.desc: ProcessUpdateRotationChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, ProcessUpdateRotationChange, TestSize.Level1)
{
    DisplayId defaultDisplayId = 0;
    sptr<DisplayInfo> displayInfo = nullptr;
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap;
    DisplayStateChangeType type = DisplayStateChangeType::BEFORE_SUSPEND;
    ASSERT_NE(nullptr, ssm_);
    ssm_->ProcessUpdateRotationChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    displayInfo = sptr<DisplayInfo>::MakeSptr();
    ASSERT_NE(nullptr, displayInfo);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest6";
    sessionInfo.abilityName_ = "UpdateAvoidArea";
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    ssm_->ProcessUpdateRotationChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    ssm_->ProcessUpdateRotationChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    sceneSession->SetSessionState(SessionState::STATE_INACTIVE);
    ssm_->ProcessUpdateRotationChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    WSRectF bounds = { 0, 0, 0, 0 };
    sceneSession->SetBounds(bounds);
    displayInfo->width_ = 0;
    displayInfo->height_ = 0;
    Rotation rotation = Rotation::ROTATION_0;
    sceneSession->SetRotation(rotation);
    displayInfo->SetRotation(rotation);
    ssm_->ProcessUpdateRotationChange(defaultDisplayId, displayInfo, displayInfoMap, type);
}

/**
 * @tc.name: FlushUIParams
 * @tc.desc: FlushUIParams
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, FlushUIParams, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "FlushUIParams";
    sessionInfo.screenId_ = 1;
    ScreenId screenId = 2;
    std::unordered_map<int32_t, SessionUIParam> uiParams;
    uiParams.clear();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->FlushUIParams(screenId, std::move(uiParams));
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ssm_->FlushUIParams(screenId, std::move(uiParams));
}

/**
 * @tc.name: FlushUIParams01
 * @tc.desc: FlushUIParams
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, FlushUIParams01, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "FlushUIParams01";
    sessionInfo.screenId_ = 2;
    ScreenId screenId = 2;
    std::unordered_map<int32_t, SessionUIParam> uiParams;
    uiParams.clear();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->persistentId_ = 1;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    SessionUIParam sessionUIParam;
    uiParams.insert(std::make_pair(1, sessionUIParam));
    ssm_->FlushUIParams(screenId, std::move(uiParams));
    sessionInfo.screenId_ = -1ULL;
    ssm_->FlushUIParams(screenId, std::move(uiParams));
}

/**
 * @tc.name: FlushUIParams02
 * @tc.desc: FlushUIParams Multi-screen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, FlushUIParams02, Function | SmallTest | Level3)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "FlushUIParams02";
    sessionInfo.screenId_ = 2;
    ScreenId screenId = 2;
    std::unordered_map<int32_t, SessionUIParam> uiParams;
    uiParams.clear();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->persistentId_ = 1;
    sceneSession->dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    SessionUIParam sessionUIParam;
    uiParams.insert(std::make_pair(1, sessionUIParam));
    ssm_->FlushUIParams(screenId, std::move(uiParams));
    sessionInfo.screenId_ = -1ULL;
    ssm_->FlushUIParams(screenId, std::move(uiParams));
    ASSERT_NE(sceneSession->dirtyFlags_, 0);
}

/**
 * @tc.name: FlushUIParams03
 * @tc.desc: FlushUIParams keyboard
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, FlushUIParams03, Function | SmallTest | Level3)
{
    SessionInfo callingSessionInfo;
    callingSessionInfo.bundleName_ = "SceneSessionManagerTest7";
    callingSessionInfo.abilityName_ = "FlushUIParams03";
    callingSessionInfo.screenId_ = 2;
    ScreenId screenId = 2;
    std::unordered_map<int32_t, SessionUIParam> uiParams;
    uiParams.clear();
    sptr<SceneSession> callingSession = sptr<SceneSession>::MakeSptr(callingSessionInfo, nullptr);
    callingSession->persistentId_ = 1;

    SessionInfo keyboardSessionInfo;
    keyboardSessionInfo.abilityName_ = "keyboardSession";
    keyboardSessionInfo.bundleName_ = "keyboardSession";
    sptr<KeyboardSession> keyboardSession = sptr<KeyboardSession>::MakeSptr(keyboardSessionInfo, nullptr, nullptr);
    keyboardSession->property_->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    keyboardSession->property_->SetCallingSessionId(1);
    keyboardSession->persistentId_ = 3;
    keyboardSession->SetIsSystemKeyboard(false);
    keyboardSession->SetScreenId(2);

    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert({1, callingSession});
    ssm_->sceneSessionMap_.insert({3, keyboardSession});
    SessionUIParam callingSessionUIParam;
    SessionUIParam keyboardSessionUIParam;
    uiParams.insert(std::make_pair(1, callingSessionUIParam));
    uiParams.insert(std::make_pair(3, keyboardSessionUIParam));
    keyboardSession->stateChanged_ = true;
    ssm_->FlushUIParams(screenId, std::move(uiParams));
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(false, keyboardSession->stateChanged_);
    
    uiParams.clear();
    uiParams.insert(std::make_pair(1, callingSessionUIParam));
    uiParams.insert(std::make_pair(3, keyboardSessionUIParam));
    keyboardSession->SetScreenId(999);
    keyboardSession->stateChanged_ = true;
    ssm_->FlushUIParams(screenId, std::move(uiParams));
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(true, keyboardSession->stateChanged_);
}

/**
 * @tc.name: RegisterIAbilityManagerCollaborator
 * @tc.desc: RegisterIAbilityManagerCollaborator
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, RegisterIAbilityManagerCollaborator, TestSize.Level1)
{
    int32_t type = 0;
    sptr<AAFwk::IAbilityManagerCollaborator> impl = nullptr;
    ASSERT_NE(nullptr, ssm_);
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    auto ret = ssm_->RegisterIAbilityManagerCollaborator(type, impl);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: ProcessBackEvent
 * @tc.desc: ProcessBackEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, ProcessBackEvent, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "FlushUIParams01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = true;
    auto ret = ssm_->ProcessBackEvent();
    EXPECT_EQ(ret, WSError::WS_OK);
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = false;
    sessionInfo.isSystem_ = true;
    ssm_->rootSceneProcessBackEventFunc_ = nullptr;
    ret = ssm_->ProcessBackEvent();
    EXPECT_EQ(ret, WSError::WS_OK);
    RootSceneProcessBackEventFunc func = []() {};
    ssm_->rootSceneProcessBackEventFunc_ = func;
    ASSERT_NE(nullptr, ssm_->rootSceneProcessBackEventFunc_);
    ret = ssm_->ProcessBackEvent();
    EXPECT_EQ(ret, WSError::WS_OK);
    sessionInfo.isSystem_ = false;
    ret = ssm_->ProcessBackEvent();
    EXPECT_EQ(ret, WSError::WS_OK);
    ssm_->rootSceneProcessBackEventFunc_ = nullptr;
    ret = ssm_->ProcessBackEvent();
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: DestroySpecificSession
 * @tc.desc: DestroySpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, DestroySpecificSession, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->remoteObjectMap_.clear();
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(nullptr, remoteObject);
    ssm_->DestroySpecificSession(remoteObject);
    ssm_->remoteObjectMap_.insert(std::make_pair(remoteObject, 1));
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "DestroySpecificSession";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->DestroySpecificSession(remoteObject);
    sceneSession = nullptr;
    ssm_->DestroySpecificSession(remoteObject);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSessionWithDetachCallback
 * @tc.desc: DestroyAndDisconnectSpecificSessionWithDetachCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, DestroyAndDisconnectSpecificSessionWithDetachCallback, TestSize.Level0)
{
    int32_t persistentId = 1;
    sptr<IRemoteObject> callback = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(nullptr, callback);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "DestroyAndDisconnectSpecificSessionWithDetachCallback";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetCallingUid(1);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    auto ret = ssm_->DestroyAndDisconnectSpecificSessionWithDetachCallback(persistentId, callback);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSessionInner
 * @tc.desc: DestroyAndDisconnectSpecificSessionInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, DestroyAndDisconnectSpecificSessionInner, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "DestroyAndDisconnectSpecificSessionInner";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sceneSession->SetParentPersistentId(2);
    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo1.abilityName_ = "ParentSceneSession";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession1));
    auto ret = ssm_->DestroyAndDisconnectSpecificSessionInner(1);
    EXPECT_EQ(ret, WSError::WS_OK);
    sceneSession1 = nullptr;
    ret = ssm_->DestroyAndDisconnectSpecificSessionInner(2);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSessionInner01
 * @tc.desc: DestroyAndDisconnectSpecificSessionInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, DestroyAndDisconnectSpecificSessionInner01, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "DestroyAndDisconnectSpecificSessionInner";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_TOAST);
    sceneSession->SetParentPersistentId(2);
    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo1.abilityName_ = "ParentSceneSession";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession1));
    auto ret = ssm_->DestroyAndDisconnectSpecificSessionInner(1);
    EXPECT_EQ(ret, WSError::WS_OK);
    sceneSession1 = nullptr;
    ret = ssm_->DestroyAndDisconnectSpecificSessionInner(2);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSessionInner02
 * @tc.desc: DestroyAndDisconnectSpecificSessionInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, DestroyAndDisconnectSpecificSessionInner02, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "DestroyAndDisconnectSpecificSessionInner";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sceneSession->SetParentPersistentId(2);
    SessionInfo sessionInfo1;
    sessionInfo1.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo1.abilityName_ = "ParentSceneSession";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession1));
    auto ret = ssm_->DestroyAndDisconnectSpecificSessionInner(1);
    EXPECT_EQ(ret, WSError::WS_OK);
    sceneSession1 = nullptr;
    ret = ssm_->DestroyAndDisconnectSpecificSessionInner(2);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSessionInner03
 * @tc.desc: DestroyAndDisconnectSpecificSessionInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, DestroyAndDisconnectSpecificSessionInner03, TestSize.Level0)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "DestroyAndDisconnectSpecificSessionInner03";
    sptr<SceneSession> floatSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    floatSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    floatSession->persistentId_ = 1;
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    subSession->persistentId_ = 2;
    subSession->SetParentSession(floatSession);
    floatSession->subSession_.push_back(subSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, floatSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, subSession));
    auto ret = ssm_->DestroyAndDisconnectSpecificSessionInner(1);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_EQ(ssm_->sceneSessionMap_.find(2), ssm_->sceneSessionMap_.end());
    ret = ssm_->DestroyAndDisconnectSpecificSessionInner(2);
    EXPECT_EQ(ret, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: CheckPiPPriority
 * @tc.desc: CheckPiPPriority
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, CheckPiPPriority, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "CheckPiPPriority";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    PiPTemplateInfo pipTemplateInfo;
    pipTemplateInfo.priority = 0;
    sceneSession->pipTemplateInfo_.priority = 1;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    auto ret = ssm_->CheckPiPPriority(pipTemplateInfo);
    EXPECT_EQ(ret, false);
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    ret = ssm_->CheckPiPPriority(pipTemplateInfo);
    EXPECT_EQ(ret, false);
    pipTemplateInfo.priority = 1;
    ret = ssm_->CheckPiPPriority(pipTemplateInfo);
    EXPECT_EQ(ret, true);
    sceneSession->property_->SetWindowMode(WindowMode::WINDOW_MODE_FLOATING);
    ret = ssm_->CheckPiPPriority(pipTemplateInfo);
    EXPECT_EQ(ret, true);
    sceneSession = nullptr;
    ret = ssm_->CheckPiPPriority(pipTemplateInfo);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: IsSessionVisibleForeground
 * @tc.desc: IsSessionVisibleForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, IsSessionVisibleForeground, TestSize.Level1)
{
    sptr<SceneSession> session = nullptr;
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->IsSessionVisibleForeground(session);
    EXPECT_EQ(ret, false);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "IsSessionVisibleForeground";
    session = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, session);
    session->isVisible_ = false;
    ret = ssm_->IsSessionVisibleForeground(session);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: UnregisterWindowManagerAgent
 * @tc.desc: UnregisterWindowManagerAgent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, UnregisterWindowManagerAgent, TestSize.Level1)
{
    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_SYSTEM_BAR;
    sptr<IWindowManagerAgent> windowManagerAgent = nullptr;
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->UnregisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_GESTURE_NAVIGATION_ENABLED;
    ret = ssm_->UnregisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WATER_MARK_FLAG;
    ret = ssm_->UnregisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
    type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_VISIBILITY;
    ret = ssm_->UnregisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PERMISSION);
    type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_DRAWING_STATE;
    ret = ssm_->UnregisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PERMISSION);
    type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_VISIBLE_WINDOW_NUM;
    ret = ssm_->UnregisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PERMISSION);
    type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    ret = ssm_->UnregisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PERMISSION);
    type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_MODE;
    ret = ssm_->UnregisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PERMISSION);
    type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_WINDOW_UPDATE;
    ret = ssm_->UnregisterWindowManagerAgent(type, windowManagerAgent);
    EXPECT_EQ(ret, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: FindSessionByAffinity
 * @tc.desc: FindSessionByAffinity
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, FindSessionByAffinity, TestSize.Level1)
{
    std::string affinity = "";
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->FindSessionByAffinity(affinity);
    EXPECT_EQ(ret, nullptr);
    affinity = "Test";
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "FindSessionByAffinity";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetCollaboratorType(CollaboratorType::DEFAULT_TYPE);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ret = ssm_->FindSessionByAffinity(affinity);
    EXPECT_EQ(ret, nullptr);
    sceneSession->SetCollaboratorType(CollaboratorType::OTHERS_TYPE);
    sceneSession->sessionInfo_.sessionAffinity = "Test";
    ret = ssm_->FindSessionByAffinity(affinity);
    EXPECT_EQ(ret, sceneSession);
}

/**
 * @tc.name: FindSessionByAffinity01
 * @tc.desc: FindSessionByAffinity
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, FindSessionByAffinity01, TestSize.Level1)
{
    std::string affinity = "Test";
    sptr<SceneSession> sceneSession = nullptr;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    auto ret = ssm_->FindSessionByAffinity(affinity);
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name: ProcessUpdateRotationChange01
 * @tc.desc: ProcessUpdateRotationChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, ProcessUpdateRotationChange01, TestSize.Level1)
{
    DisplayId defaultDisplayId = 0;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    ASSERT_NE(nullptr, displayInfo);
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap;
    DisplayStateChangeType type = DisplayStateChangeType::BEFORE_SUSPEND;
    sptr<SceneSession> sceneSession = nullptr;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->ProcessUpdateRotationChange(defaultDisplayId, displayInfo, displayInfoMap, type);
}

/**
 * @tc.name: ProcessUpdateRotationChange02
 * @tc.desc: ProcessUpdateRotationChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, ProcessUpdateRotationChange02, TestSize.Level1)
{
    DisplayId defaultDisplayId = 0;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    ASSERT_NE(nullptr, displayInfo);
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap;
    DisplayStateChangeType type = DisplayStateChangeType::BEFORE_SUSPEND;
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "UpdateAvoidArea";
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    sceneSession->SetSessionState(SessionState::STATE_INACTIVE);
    WSRectF bounds = { 0, 0, 0, 0 };
    sceneSession->SetBounds(bounds);
    displayInfo->width_ = 0;
    displayInfo->height_ = 0;
    sceneSession->SetRotation(Rotation::ROTATION_0);
    displayInfo->SetRotation(Rotation::ROTATION_90);
    ssm_->ProcessUpdateRotationChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    displayInfo->height_ = 1;
    ssm_->ProcessUpdateRotationChange(defaultDisplayId, displayInfo, displayInfoMap, type);
    displayInfo->width_ = 1;
    ssm_->ProcessUpdateRotationChange(defaultDisplayId, displayInfo, displayInfoMap, type);
}

/**
 * @tc.name: SetSkipSelfWhenShowOnVirtualScreen
 * @tc.desc: SetSkipSelfWhenShowOnVirtualScreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetSkipSelfWhenShowOnVirtualScreen, TestSize.Level1)
{
    uint64_t surfaceNodeId = 0;
    bool isSkip = true;
    ASSERT_NE(nullptr, ssm_);
    ssm_->skipSurfaceNodeIds_.clear();
    ssm_->SetSkipSelfWhenShowOnVirtualScreen(surfaceNodeId, isSkip);
    ssm_->skipSurfaceNodeIds_.push_back(surfaceNodeId);
    ssm_->SetSkipSelfWhenShowOnVirtualScreen(surfaceNodeId, isSkip);
}

/**
 * @tc.name: SetSkipSelfWhenShowOnVirtualScreen01
 * @tc.desc: SetSkipSelfWhenShowOnVirtualScreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetSkipSelfWhenShowOnVirtualScreen01, TestSize.Level1)
{
    uint64_t surfaceNodeId = 0;
    bool isSkip = false;
    ASSERT_NE(nullptr, ssm_);
    ssm_->skipSurfaceNodeIds_.clear();
    ssm_->SetSkipSelfWhenShowOnVirtualScreen(surfaceNodeId, isSkip);
    ssm_->skipSurfaceNodeIds_.push_back(surfaceNodeId);
    ssm_->SetSkipSelfWhenShowOnVirtualScreen(surfaceNodeId, isSkip);
}

/**
 * @tc.name: GetMainWindowInfos
 * @tc.desc: GetMainWindowInfos
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, GetMainWindowInfos, TestSize.Level1)
{
    int32_t topNum = 1;
    std::vector<MainWindowInfo> topNInfo;
    topNInfo.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "GetMainWindowInfos";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    auto ret = ssm_->GetMainWindowInfos(topNum, topNInfo);
    EXPECT_EQ(ret, WMError::WM_OK);
    sceneSession->isVisible_ = false;
    ret = ssm_->GetMainWindowInfos(topNum, topNInfo);
    EXPECT_EQ(ret, WMError::WM_OK);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ret = ssm_->GetMainWindowInfos(topNum, topNInfo);
    EXPECT_EQ(ret, WMError::WM_OK);
    sceneSession->isVisible_ = true;
    ret = ssm_->GetMainWindowInfos(topNum, topNInfo);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: WindowLayerInfoChangeCallback
 * @tc.desc: WindowLayerInfoChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, WindowLayerInfoChangeCallback, TestSize.Level1)
{
    std::shared_ptr<RSOcclusionData> occlusiontionData = nullptr;
    ASSERT_NE(nullptr, ssm_);
    ssm_->WindowLayerInfoChangeCallback(occlusiontionData);
    VisibleData visibleData;
    visibleData.push_back(std::make_pair(0, WINDOW_LAYER_INFO_TYPE::ALL_VISIBLE));
    visibleData.push_back(std::make_pair(1, WINDOW_LAYER_INFO_TYPE::SEMI_VISIBLE));
    visibleData.push_back(std::make_pair(2, WINDOW_LAYER_INFO_TYPE::INVISIBLE));
    visibleData.push_back(std::make_pair(3, WINDOW_LAYER_INFO_TYPE::WINDOW_LAYER_DYNAMIC_STATUS));
    visibleData.push_back(std::make_pair(4, WINDOW_LAYER_INFO_TYPE::WINDOW_LAYER_STATIC_STATUS));
    visibleData.push_back(std::make_pair(5, WINDOW_LAYER_INFO_TYPE::WINDOW_LAYER_UNKNOWN_TYPE));
    occlusiontionData = std::make_shared<RSOcclusionData>(visibleData);
    ASSERT_NE(nullptr, occlusiontionData);
    ssm_->WindowLayerInfoChangeCallback(occlusiontionData);
}

/**
 * @tc.name: NotifySessionMovedToFront
 * @tc.desc: NotifySessionMovedToFront
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, NotifySessionMovedToFront, TestSize.Level1)
{
    int32_t persistentId = 1;
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "GetMainWindowInfos";
    sessionInfo.isSystem_ = false;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->listenerController_ = std::make_shared<SessionListenerController>();
    ASSERT_NE(nullptr, ssm_->listenerController_);
    sceneSession->sessionInfo_.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    ASSERT_NE(nullptr, sceneSession->sessionInfo_.abilityInfo);
    sceneSession->sessionInfo_.abilityInfo->excludeFromMissions = false;
    ssm_->sceneSessionMap_.insert(std::make_pair(persistentId, sceneSession));
    ssm_->NotifySessionMovedToFront(persistentId);
    sceneSession->sessionInfo_.abilityInfo->excludeFromMissions = true;
    ssm_->NotifySessionMovedToFront(persistentId);
    sceneSession->sessionInfo_.abilityInfo = nullptr;
    ssm_->NotifySessionMovedToFront(persistentId);
    sceneSession->sessionInfo_.isSystem_ = true;
    ssm_->NotifySessionMovedToFront(persistentId);
    ssm_->listenerController_ = nullptr;
    ssm_->NotifySessionMovedToFront(persistentId);
}

/**
 * @tc.name: ProcessVirtualPixelRatioChange02
 * @tc.desc: ProcessVirtualPixelRatioChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, ProcessVirtualPixelRatioChange02, TestSize.Level1)
{
    DisplayId defaultDisplayId = 0;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap;
    DisplayStateChangeType type = DisplayStateChangeType::BEFORE_SUSPEND;
    ASSERT_NE(nullptr, displayInfo);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "ProcessVirtualPixelRatioChange02";
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    sptr<SceneSession> sceneSession1 = nullptr;
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession1));
    ssm_->ProcessVirtualPixelRatioChange(defaultDisplayId, displayInfo, displayInfoMap, type);
}

/**
 * @tc.name: ProcessVirtualPixelRatioChange03
 * @tc.desc: ProcessVirtualPixelRatioChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, ProcessVirtualPixelRatioChange03, TestSize.Level1)
{
    DisplayId defaultDisplayId = 0;
    sptr<DisplayInfo> displayInfo = sptr<DisplayInfo>::MakeSptr();
    std::map<DisplayId, sptr<DisplayInfo>> displayInfoMap;
    DisplayStateChangeType type = DisplayStateChangeType::BEFORE_SUSPEND;
    ASSERT_NE(nullptr, displayInfo);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "ProcessVirtualPixelRatioChange03";
    sessionInfo.isSystem_ = false;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    sceneSession1->SetSessionState(SessionState::STATE_ACTIVE);
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession1));
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession2);
    sceneSession2->SetSessionState(SessionState::STATE_INACTIVE);
    ssm_->sceneSessionMap_.insert(std::make_pair(3, sceneSession2));
    ssm_->ProcessVirtualPixelRatioChange(defaultDisplayId, displayInfo, displayInfoMap, type);
}

/**
 * @tc.name: ProcessBackEvent01
 * @tc.desc: ProcessBackEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, ProcessBackEvent01, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "ProcessBackEvent01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = true;
    auto ret = ssm_->ProcessBackEvent();
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: ProcessBackEvent02
 * @tc.desc: ProcessBackEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, ProcessBackEvent02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "ProcessBackEvent02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->sessionInfo_.isSystem_ = true;
    ASSERT_NE(nullptr, ssm_);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = false;
    ssm_->rootSceneProcessBackEventFunc_ = nullptr;
    auto ret = ssm_->ProcessBackEvent();
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: ProcessBackEvent03
 * @tc.desc: ProcessBackEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, ProcessBackEvent03, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "ProcessBackEvent03";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->sessionInfo_.isSystem_ = true;
    ASSERT_NE(nullptr, ssm_);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = false;
    RootSceneProcessBackEventFunc func = []() {};
    ssm_->rootSceneProcessBackEventFunc_ = func;
    ASSERT_NE(nullptr, ssm_->rootSceneProcessBackEventFunc_);
    auto ret = ssm_->ProcessBackEvent();
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: ProcessBackEvent02
 * @tc.desc: ProcessBackEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, ProcessBackEvent04, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "ProcessBackEvent04";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->sessionInfo_.isSystem_ = false;
    ASSERT_NE(nullptr, ssm_);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = false;
    ssm_->rootSceneProcessBackEventFunc_ = nullptr;
    auto ret = ssm_->ProcessBackEvent();
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: ProcessBackEvent05
 * @tc.desc: ProcessBackEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, ProcessBackEvent05, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "ProcessBackEvent03";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->sessionInfo_.isSystem_ = false;
    ASSERT_NE(nullptr, ssm_);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = false;
    RootSceneProcessBackEventFunc func = []() {};
    ssm_->rootSceneProcessBackEventFunc_ = func;
    ASSERT_NE(nullptr, ssm_->rootSceneProcessBackEventFunc_);
    auto ret = ssm_->ProcessBackEvent();
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: GetWindowVisibilityChangeInfo
 * @tc.desc: GetWindowVisibilityChangeInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, GetWindowVisibilityChangeInfo, TestSize.Level1)
{
    std::vector<std::pair<uint64_t, WindowVisibilityState>> currVisibleData;
    std::vector<std::pair<uint64_t, WindowVisibilityState>> visibilitychangeInfos;
    currVisibleData.emplace_back(2, WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    currVisibleData.emplace_back(4, WindowVisibilityState::WINDOW_LAYER_STATE_MAX);
    currVisibleData.emplace_back(5, WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
    currVisibleData.emplace_back(7, WindowVisibilityState::WINDOW_LAYER_STATE_MAX);
    currVisibleData.emplace_back(9, WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
    ASSERT_NE(nullptr, ssm_);
    ssm_->lastVisibleData_.emplace_back(1, WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    ssm_->lastVisibleData_.emplace_back(4, WindowVisibilityState::WINDOW_VISIBILITY_STATE_PARTICALLY_OCCLUSION);
    ssm_->lastVisibleData_.emplace_back(5, WindowVisibilityState::WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
    ssm_->lastVisibleData_.emplace_back(6, WindowVisibilityState::WINDOW_LAYER_STATE_MAX);
    visibilitychangeInfos = ssm_->GetWindowVisibilityChangeInfo(currVisibleData);
    ASSERT_EQ(visibilitychangeInfos.size(), 7);
}

/**
 * @tc.name: NotifySessionMovedToFront01
 * @tc.desc: NotifySessionMovedToFront
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, NotifySessionMovedToFront01, TestSize.Level1)
{
    int32_t persistentId = 1;
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "NotifySessionMovedToFront01";
    sessionInfo.isSystem_ = false;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->listenerController_ = std::make_shared<SessionListenerController>();
    ASSERT_NE(nullptr, ssm_->listenerController_);
    sceneSession->sessionInfo_.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    ASSERT_NE(nullptr, sceneSession->sessionInfo_.abilityInfo);
    sceneSession->sessionInfo_.abilityInfo->excludeFromMissions = true;
    ssm_->sceneSessionMap_.insert(std::make_pair(persistentId, sceneSession));
    ssm_->NotifySessionMovedToFront(persistentId);
}

/**
 * @tc.name: NotifySessionMovedToFront02
 * @tc.desc: NotifySessionMovedToFront
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, NotifySessionMovedToFront02, TestSize.Level1)
{
    int32_t persistentId = 1;
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "NotifySessionMovedToFront02";
    sessionInfo.isSystem_ = false;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->listenerController_ = std::make_shared<SessionListenerController>();
    ASSERT_NE(nullptr, ssm_->listenerController_);
    sceneSession->sessionInfo_.abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    ASSERT_NE(nullptr, sceneSession->sessionInfo_.abilityInfo);
    sceneSession->sessionInfo_.abilityInfo->excludeFromMissions = false;
    ssm_->sceneSessionMap_.insert(std::make_pair(persistentId, sceneSession));
    ssm_->NotifySessionMovedToFront(persistentId);
}

/**
 * @tc.name: NotifySessionMovedToFront03
 * @tc.desc: NotifySessionMovedToFront
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, NotifySessionMovedToFront03, TestSize.Level1)
{
    int32_t persistentId = 1;
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "NotifySessionMovedToFront03";
    sessionInfo.isSystem_ = false;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->listenerController_ = std::make_shared<SessionListenerController>();
    ASSERT_NE(nullptr, ssm_->listenerController_);
    sceneSession->sessionInfo_.abilityInfo = nullptr;
    ssm_->NotifySessionMovedToFront(persistentId);
}

/**
 * @tc.name: NotifySessionMovedToFront04
 * @tc.desc: NotifySessionMovedToFront
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, NotifySessionMovedToFront04, TestSize.Level1)
{
    int32_t persistentId = 1;
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "NotifySessionMovedToFront04";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->listenerController_ = std::make_shared<SessionListenerController>();
    ASSERT_NE(nullptr, ssm_->listenerController_);
    sceneSession->sessionInfo_.isSystem_ = true;
    ssm_->NotifySessionMovedToFront(persistentId);
}

/**
 * @tc.name: NotifySessionMovedToFront05
 * @tc.desc: NotifySessionMovedToFront
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, NotifySessionMovedToFront05, TestSize.Level1)
{
    int32_t persistentId = 1;
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "NotifySessionMovedToFront05";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->listenerController_ = nullptr;
    ssm_->NotifySessionMovedToFront(persistentId);
}

/**
 * @tc.name: UpdateNormalSessionAvoidArea02
 * @tc.desc: UpdateNormalSessionAvoidArea
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, UpdateNormalSessionAvoidArea02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "UpdateNormalSessionAvoidArea02";
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->GetLayoutController()->SetSessionRect({ 1, 1, 1, 1 });
    int32_t persistentId = 1;
    bool needUpdate = true;
    ASSERT_NE(nullptr, ssm_);
    ssm_->avoidAreaListenerSessionSet_.clear();
    ssm_->avoidAreaListenerSessionSet_.insert(persistentId);
    ssm_->UpdateNormalSessionAvoidArea(persistentId, sceneSession, needUpdate);
}

/**
 * @tc.name: SetSessionSnapshotSkipForAppProcess
 * @tc.desc: SceneSesionManager SetSessionSnapshotSkipForAppProcess
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetSessionSnapshotSkipForAppProcess, TestSize.Level1)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, nullptr);
    sceneSession->SetCallingPid(1000);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    sceneSession->SetSurfaceNode(surfaceNode);
    ssm_->SetSessionSnapshotSkipForAppProcess(sceneSession);
    ASSERT_EQ(sceneSession->GetSessionProperty()->GetSnapshotSkip(), false);

    ssm_->snapshotSkipPidSet_.insert(1000);
    ssm_->SetSessionSnapshotSkipForAppProcess(sceneSession);
    ASSERT_EQ(sceneSession->GetSessionProperty()->GetSnapshotSkip(), true);
    ssm_->snapshotSkipPidSet_.erase(1000);
}

/**
 * @tc.name: TestReportCorrectScreenFoldStatusChangeEvent
 * @tc.desc: Test whether report the correct screen fold status events
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, TestReportCorrectScreenFoldStatusChangeEvent, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneSessionManagerTest: TestReportCorrectScreenFoldStatusChangeEvent start";
    ScreenFoldData screenFoldData1;
    screenFoldData1.currentScreenFoldStatus_ = 1;          // 1: current screen fold status
    screenFoldData1.nextScreenFoldStatus_ = 3;             // 3: next screen fold status
    screenFoldData1.currentScreenFoldStatusDuration_ = 18; // 18: current duration
    screenFoldData1.postureAngle_ = 47.1f;                 // 47.1: posture angle (type: float)
    screenFoldData1.screenRotation_ = 1;                   // 1: screen rotation
    screenFoldData1.typeCThermal_ = 3000;                  // 3000: typec port thermal
    screenFoldData1.focusedPackageName_ = "Developer Test: (1, 3, 18, 47.1, 1, 3000)";
    WMError result = ssm_->CheckAndReportScreenFoldStatus(screenFoldData1);
    ASSERT_EQ(result, WMError::WM_DO_NOTHING); // not report half-fold event until next change

    ScreenFoldData screenFoldData2;
    screenFoldData2.currentScreenFoldStatus_ = 3;          // 3: current screen fold status
    screenFoldData2.nextScreenFoldStatus_ = 2;             // 2: next screen fold status
    screenFoldData2.currentScreenFoldStatusDuration_ = 20; // 20: current duration
    screenFoldData2.postureAngle_ = 143.7f;                // 143.7: posture angle (type: float)
    screenFoldData2.screenRotation_ = 2;                   // 2: screen rotation
    screenFoldData2.typeCThermal_ = 3005;                  // 3005: typec port thermal
    screenFoldData2.focusedPackageName_ = "Developer Test: (3, 2, 20, 143.7, 2, 3005)";
    result = ssm_->CheckAndReportScreenFoldStatus(screenFoldData2);
    ASSERT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name: TestReportIncompleteScreenFoldStatusChangeEvent
 * @tc.desc: Test whether block the incomplete screen fold status events
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, TestReportIncompleteScreenFoldStatusChangeEvent, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneSessionManagerTest: TestReportIncompleteScreenFoldStatusChangeEvent start";
    // screen fold status changes from -1: invalid to 3: half_fold, duration = 0, angle = 67.0, rotation = 0
    std::vector<std::string> screenFoldInfo{ "-1", "3", "0", "67.0", "0" };
    WMError result = ssm_->ReportScreenFoldStatusChange(screenFoldInfo);
    ASSERT_EQ(result, WMError::WM_DO_NOTHING);

    screenFoldInfo.clear();
    result = ssm_->ReportScreenFoldStatusChange(screenFoldInfo);
    ASSERT_EQ(result, WMError::WM_DO_NOTHING);

    // screen fold status changes from 2: folded to 3: half_fold, duration = 0, angle = 67.0, rotation = 0
    screenFoldInfo = { "2", "3", "0", "67.0", "0" };
    result = ssm_->ReportScreenFoldStatusChange(screenFoldInfo);
    ASSERT_EQ(result, WMError::WM_DO_NOTHING);

    // screen fold status changes from 3: half_fold to 1: expand, duration = 18, angle = 147.3, rotation = 2
    screenFoldInfo = { "3", "1", "18", "147.3", "2" };
    result = ssm_->ReportScreenFoldStatusChange(screenFoldInfo);
    ASSERT_EQ(result, WMError::WM_DO_NOTHING);
}

/**
 * @tc.name: SetAppForceLandscapeConfig
 * @tc.desc: SceneSesionManager SetAppForceLandscapeConfig
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetAppForceLandscapeConfig, TestSize.Level1)
{
    std::string bundleName = "SetAppForceLandscapeConfig";
    AppForceLandscapeConfig config = { 0, false, false, {}, {}, {}, false, false, false, false };
    WSError result = ssm_->SetAppForceLandscapeConfig(bundleName, config);
    ASSERT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: SetAppForceLandscapeConfig01
 * @tc.desc: SetAppForceLandscapeConfig_ShouldReturnNullptrError_WhenBundleNameIsEmpty
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetAppForceLandscapeConfig01, TestSize.Level1)
{
    std::string bundleName = "";
    AppForceLandscapeConfig config;
    WSError result = ssm_->SetAppForceLandscapeConfig(bundleName, config);
    EXPECT_EQ(result, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: SetAppForceLandscapeConfig02
 * @tc.desc: SetAppForceLandscapeConfig_ShouldUpdateConfig_WhenBundleNameIsValid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetAppForceLandscapeConfig02, TestSize.Level1)
{
    std::string bundleName = "com.example.app";
    AppForceLandscapeConfig config;
    config.mode_ = 5; // 5: FORCE_SPLIT_MODE
    config.supportSplit_ = 5;

    WSError result = ssm_->SetAppForceLandscapeConfig(bundleName, config);
    EXPECT_EQ(result, WSError::WS_OK);
    EXPECT_EQ(ssm_->appForceLandscapeMap_[bundleName].mode_, 5);
    EXPECT_EQ(ssm_->appForceLandscapeMap_[bundleName].supportSplit_, 5);
}

/**
 * @tc.name: SetAppForceLandscapeConfig03
 * @tc.desc: SetAppForceLandscapeConfig_ShouldUpdateConfig_WhenBundleNameIsValid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetAppForceLandscapeConfig03, TestSize.Level1)
{
    std::string bundleName = "com.example.app";
    AppForceLandscapeConfig preConfig;
    preConfig.mode_ = 0;
    preConfig.supportSplit_ = -1;
    ssm_->appForceLandscapeMap_[bundleName] = preConfig;

    AppForceLandscapeConfig config;
    config.mode_ = 5; // 5: FORCE_SPLIT_MODE
    config.supportSplit_ = 5;

    WSError result = ssm_->SetAppForceLandscapeConfig(bundleName, config);
    EXPECT_EQ(result, WSError::WS_OK);
    EXPECT_EQ(ssm_->appForceLandscapeMap_[bundleName].mode_, 5);
    EXPECT_EQ(ssm_->appForceLandscapeMap_[bundleName].supportSplit_, 5);
}

/**
 * @tc.name: GetAppForceLandscapeConfig
 * @tc.desc: SceneSesionManager GetAppForceLandscapeConfig
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, GetAppForceLandscapeConfig, TestSize.Level1)
{
    std::string bundleName = "GetAppForceLandscapeConfig";
    AppForceLandscapeConfig config = ssm_->GetAppForceLandscapeConfig(bundleName);
    EXPECT_EQ(config.mode_, 0);
    EXPECT_EQ(config.supportSplit_, -1);
}

/**
 * @tc.name: SetSessionWatermarkForAppProcess
 * @tc.desc: SceneSesionManager SetSessionWatermarkForAppProcess
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetSessionWatermarkForAppProcess, TestSize.Level0)
{
    SessionInfo info;
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, nullptr);
    sceneSession->SetCallingPid(1);
    ASSERT_FALSE(ssm_->SetSessionWatermarkForAppProcess(sceneSession));
    ssm_->processWatermarkPidMap_.insert({ 1, "test" });
    ASSERT_TRUE(ssm_->SetSessionWatermarkForAppProcess(sceneSession));
    ssm_->processWatermarkPidMap_.erase(1);
}

/**
 * @tc.name: CloseTargetFloatWindow
 * @tc.desc: SceneSesionManager CloseTargetFloatWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, CloseTargetFloatWindow, TestSize.Level1)
{
    std::string bundleName = "testClose";
    auto result = ssm_->CloseTargetFloatWindow(bundleName);
    ASSERT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name: CloseTargetPiPWindow
 * @tc.desc: SceneSesionManager CloseTargetPiPWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, CloseTargetPiPWindow, TestSize.Level1)
{
    std::string bundleName = "CloseTargetPiPWindow";
    auto result = ssm_->CloseTargetPiPWindow(bundleName);
    ASSERT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name: GetCurrentPiPWindowInfo01
 * @tc.desc: SceneSesionManager GetCurrentPiPWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, GetCurrentPiPWindowInfo01, TestSize.Level1)
{
    std::string bundleName;
    auto result = ssm_->GetCurrentPiPWindowInfo(bundleName);
    ASSERT_EQ(result, WMError::WM_OK);
    ASSERT_EQ("", bundleName);
}

/**
 * @tc.name: GetCurrentPiPWindowInfo02
 * @tc.desc: SceneSesionManager GetCurrentPiPWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, GetCurrentPiPWindowInfo02, TestSize.Level1)
{
    SessionInfo info1;
    info1.abilityName_ = "test1";
    info1.bundleName_ = "test1";
    info1.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_PIP);
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    SessionInfo info2;
    info2.abilityName_ = "test2";
    info2.bundleName_ = "test2";
    info2.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_DIALOG);
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    ASSERT_NE(nullptr, sceneSession2);

    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });
    std::string bundleName;
    auto result = ssm_->GetCurrentPiPWindowInfo(bundleName);
    ASSERT_EQ(result, WMError::WM_OK);
    ASSERT_EQ(info1.abilityName_, bundleName);
}

/**
 * @tc.name: SkipSnapshotByUserIdAndBundleNames
 * @tc.desc: SkipSnapshotByUserIdAndBundleNames
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SkipSnapshotByUserIdAndBundleNames, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    auto result = ssm_->SkipSnapshotByUserIdAndBundleNames(100, { "TestName" });
    ASSERT_EQ(result, WMError::WM_OK);
    usleep(WAIT_SYNC_FOR_SNAPSHOT_SKIP_IN_NS);
    ASSERT_NE(ssm_->snapshotSkipBundleNameSet_.find("TestName"), ssm_->snapshotSkipBundleNameSet_.end());

    result = ssm_->SkipSnapshotByUserIdAndBundleNames(100, {});
    ASSERT_EQ(result, WMError::WM_OK);
    usleep(WAIT_SYNC_FOR_SNAPSHOT_SKIP_IN_NS);
    ASSERT_EQ(ssm_->snapshotSkipBundleNameSet_.find("TestName"), ssm_->snapshotSkipBundleNameSet_.end());

    SessionInfo info1;
    info1.bundleName_ = "TestName1";
    sptr<SceneSession> sceneSession1 = ssm_->CreateSceneSession(info1, nullptr);
    SessionInfo info2;
    info1.bundleName_ = "TestName2";
    sptr<SceneSession> sceneSession2 = ssm_->CreateSceneSession(info2, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    ASSERT_NE(nullptr, sceneSession2);
    sceneSession1->SetCallingPid(1000);
    sceneSession2->SetCallingPid(1001);
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });
    ssm_->sceneSessionMap_.insert({ -1, nullptr });
    result = ssm_->SkipSnapshotByUserIdAndBundleNames(100, { "TestName1" });
    ASSERT_EQ(result, WMError::WM_OK);
    ssm_->sceneSessionMap_.erase(sceneSession1->GetPersistentId());
    ssm_->sceneSessionMap_.erase(sceneSession2->GetPersistentId());
    ssm_->sceneSessionMap_.erase(-1);
    usleep(WAIT_SYNC_FOR_TEST_END_IN_NS);
}

/**
 * @tc.name: SetSessionSnapshotSkipForAppBundleName
 * @tc.desc: SceneSesionManager SetSessionSnapshotSkipForAppBundleName
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetSessionSnapshotSkipForAppBundleName, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "TestName";
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, nullptr);
    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    sceneSession->SetSurfaceNode(surfaceNode);
    ssm_->SetSessionSnapshotSkipForAppBundleName(sceneSession);
    ASSERT_EQ(sceneSession->GetSessionProperty()->GetSnapshotSkip(), false);

    ssm_->snapshotSkipBundleNameSet_.insert("TestName");
    ssm_->SetSessionSnapshotSkipForAppBundleName(sceneSession);
    ASSERT_EQ(sceneSession->GetSessionProperty()->GetSnapshotSkip(), true);
    ssm_->snapshotSkipBundleNameSet_.erase("TestName");
}

/**
 * @tc.name: GetRootMainWindowId
 * @tc.desc: SceneSesionManager GetRootMainWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, GetRootMainWindowId, TestSize.Level1)
{
    SessionInfo info1;
    info1.abilityName_ = "test1";
    info1.bundleName_ = "test1";
    info1.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    SessionInfo info2;
    info2.abilityName_ = "test2";
    info2.bundleName_ = "test2";
    info2.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    ASSERT_NE(nullptr, sceneSession2);
    sceneSession2->SetParentSession(sceneSession1);

    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });
    int32_t hostWindowId = -1;
    auto result = ssm_->GetRootMainWindowId(sceneSession2->GetPersistentId(), hostWindowId);
    ASSERT_EQ(result, WMError::WM_OK);
    ASSERT_EQ(hostWindowId, sceneSession1->GetPersistentId());
}

/**
 * @tc.name: UpdateScreenLockStatusForApp
 * @tc.desc: SceneSesionManager UpdateScreenLockStatusForApp
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, UpdateScreenLockStatusForApp, TestSize.Level1)
{
    auto result = ssm_->UpdateScreenLockStatusForApp("", true);
    ASSERT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name: UpdateAppHookDisplayInfo001
 * @tc.desc: Test delete HookDisplayInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, UpdateAppHookDisplayInfo001, TestSize.Level1)
{
    int32_t uid = 0;
    bool enable = false;
    HookInfo hookInfo;
    hookInfo.width_ = 100;
    hookInfo.height_ = 100;
    hookInfo.density_ = 2.25;
    hookInfo.rotation_ = 0;
    hookInfo.enableHookRotation_ = false;
    auto result = ssm_->UpdateAppHookDisplayInfo(uid, hookInfo, enable);
    ASSERT_EQ(result, WMError::WM_OK);

    uid = 20221524;
    hookInfo.width_ = 0;
    result = ssm_->UpdateAppHookDisplayInfo(uid, hookInfo, enable);
    ASSERT_EQ(result, WMError::WM_OK);

    hookInfo.width_ = 100;
    hookInfo.height_ = 0;
    result = ssm_->UpdateAppHookDisplayInfo(uid, hookInfo, enable);
    ASSERT_EQ(result, WMError::WM_OK);

    hookInfo.height_ = 100;
    hookInfo.density_ = 0;
    result = ssm_->UpdateAppHookDisplayInfo(uid, hookInfo, enable);
    ASSERT_EQ(result, WMError::WM_OK);

    hookInfo.density_ = 2.25;
    result = ssm_->UpdateAppHookDisplayInfo(uid, hookInfo, enable);
    ASSERT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name: UpdateAppHookDisplayInfo002
 * @tc.desc: Test add HookDisplayInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, UpdateAppHookDisplayInfo002, TestSize.Level1)
{
    int32_t uid = 0;
    bool enable = true;
    HookInfo hookInfo;
    hookInfo.width_ = 100;
    hookInfo.height_ = 100;
    hookInfo.density_ = 2.25;
    hookInfo.rotation_ = 0;
    hookInfo.enableHookRotation_ = false;
    auto result = ssm_->UpdateAppHookDisplayInfo(uid, hookInfo, enable);
    ASSERT_EQ(result, WMError::WM_ERROR_INVALID_PARAM);

    uid = 20221524;
    hookInfo.width_ = 0;
    result = ssm_->UpdateAppHookDisplayInfo(uid, hookInfo, enable);
    ASSERT_EQ(result, WMError::WM_ERROR_INVALID_PARAM);

    hookInfo.width_ = 100;
    hookInfo.height_ = 0;
    result = ssm_->UpdateAppHookDisplayInfo(uid, hookInfo, enable);
    ASSERT_EQ(result, WMError::WM_ERROR_INVALID_PARAM);

    hookInfo.height_ = 100;
    hookInfo.density_ = 0;
    result = ssm_->UpdateAppHookDisplayInfo(uid, hookInfo, enable);
    ASSERT_EQ(result, WMError::WM_ERROR_INVALID_PARAM);

    hookInfo.density_ = 2.25;
    result = ssm_->UpdateAppHookDisplayInfo(uid, hookInfo, enable);
    ASSERT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name: IsPcWindow
 * @tc.desc: IsPcWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, IsPcWindow, TestSize.Level1)
{
    bool isPcWindow = false;
    auto result = ssm_->IsPcWindow(isPcWindow);
    ASSERT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name: IsPcOrPadFreeMultiWindowMode
 * @tc.desc: IsPcOrPadFreeMultiWindowMode
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, IsPcOrPadFreeMultiWindowMode, TestSize.Level1)
{
    bool isPcOrPadFreeMultiWindowMode = false;
    auto result = ssm_->IsPcOrPadFreeMultiWindowMode(isPcOrPadFreeMultiWindowMode);
    ASSERT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name: SetImageForRecent001
 * @tc.desc: SetImageForRecent001
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetImageForRecent001, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    info.persistentId_ = 1999;
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_WINDOW_BASE);
    auto result = ssm_->SetImageForRecent(1, ImageFit::FILL, 1);
    ASSERT_EQ(result, WMError::WM_ERROR_NULLPTR);

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    AppExecFwk::ApplicationInfo applicationInfo;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    result = ssm_->SetImageForRecent(1, ImageFit::FILL, sceneSession->GetPersistentId());
    ASSERT_EQ(result, WMError::WM_ERROR_NULLPTR);

    sceneSession->state_ = SessionState::STATE_ACTIVE;
    applicationInfo.isSystemApp = false;
    abilityInfo->applicationInfo = applicationInfo;
    sceneSession->SetAbilitySessionInfo(abilityInfo);
    result = ssm_->SetImageForRecent(1, ImageFit::FILL, sceneSession->GetPersistentId());
    ASSERT_EQ(result, WMError::WM_ERROR_NOT_SYSTEM_APP);

    applicationInfo.isSystemApp = true;
    abilityInfo->applicationInfo = applicationInfo;
    sceneSession->SetAbilitySessionInfo(abilityInfo);
    result = ssm_->SetImageForRecent(1, ImageFit::FILL, sceneSession->GetPersistentId());
    ASSERT_EQ(result, WMError::WM_ERROR_NULLPTR);
    ssm_->sceneSessionMap_.erase(sceneSession->GetPersistentId());
}

/**
 * @tc.name: IsWindowRectAutoSave
 * @tc.desc: IsWindowRectAutoSave
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, IsWindowRectAutoSave, TestSize.Level1)
{
    std::string key = "com.example.recposentryEntryAbilityabc";
    bool enabled = false;
    int persistentId = 1;
    auto result = ssm_->IsWindowRectAutoSave(key, enabled, persistentId);
    ASSERT_EQ(result, WMError::WM_ERROR_INVALID_SESSION);
}

/**
 * @tc.name: SetImageForRecent
 * @tc.desc: SetImageForRecent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetImageForRecent, TestSize.Level1)
{
    uint32_t imgResourceId = 1;
    ImageFit imageFit = ImageFit::FILL;
    int32_t persistentId = 1;
    auto result = ssm_->SetImageForRecent(imgResourceId, imageFit, persistentId);
    ASSERT_EQ(result, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: SetIsWindowRectAutoSave
 * @tc.desc: SetIsWindowRectAutoSave
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetIsWindowRectAutoSave, TestSize.Level1)
{
    std::string key = "com.example.recposentryEntryAbilityTest";
    bool enabled = true;
    std::string abilityKey = "com.example.recposentryEntryAbility";
    bool isSaveSpecifiedFlag = true;
    ssm_->SetIsWindowRectAutoSave(key, enabled, abilityKey, isSaveSpecifiedFlag);
    ASSERT_EQ(ssm_->isWindowRectAutoSaveMap_.at(key), true);
    ASSERT_EQ(ssm_->isSaveBySpecifiedFlagMap_.at(abilityKey), true);
}

/**
 * @tc.name: GetDisplayIdByWindowId01
 * @tc.desc: test function : GetDisplayIdByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, GetDisplayIdByWindowId01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession2);
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });

    DisplayId displayId = 0;
    sceneSession1->property_->SetDisplayId(displayId);

    const std::vector<uint64_t> windowIds = { 1001,
                                              sceneSession1->GetPersistentId(),
                                              sceneSession2->GetPersistentId() };
    std::unordered_map<uint64_t, DisplayId> windowDisplayIdMap;
    ASSERT_EQ(ssm_->GetDisplayIdByWindowId(windowIds, windowDisplayIdMap), WMError::WM_OK);
}

/**
 * @tc.name: GetDisplayIdByWindowId02
 * @tc.desc: Half fold
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, GetDisplayIdByWindowId02, TestSize.Level1)
{
    PcFoldScreenManager::GetInstance().UpdateFoldScreenStatus(
        0, SuperFoldStatus::HALF_FOLDED, { 0, 0, 2472, 1648 }, { 0, 1648, 2472, 1648 }, { 0, 1624, 2472, 1648 });
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession1->SetVisibilityState(WINDOW_VISIBILITY_STATE_NO_OCCLUSION);
    WSRect rect = { 0, 4000, 120, 1000 };
    sceneSession1->SetSessionRect(rect);
    sceneSession1->SetSessionGlobalRect(rect);
    sceneSession1->property_->SetDisplayId(0);
    sceneSession1->SetClientDisplayId(999);
    int32_t zOrder = 100;
    sceneSession1->SetZOrder(zOrder);
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });

    std::vector<uint64_t> windowIds;
    windowIds.emplace_back(sceneSession1->GetPersistentId());
    std::unordered_map<uint64_t, DisplayId> windowDisplayIdMap;
    ssm_->GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
    ssm_->sceneSessionMap_.clear();
    ASSERT_EQ(windowDisplayIdMap[sceneSession1->GetPersistentId()], 999);
}

/**
 * @tc.name: SetGlobalDragResizeType01
 * @tc.desc: test function : SetGlobalDragResizeType valid session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetGlobalDragResizeType01, TestSize.Level1)
{
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    ASSERT_EQ(ssm_->SetGlobalDragResizeType(dragResizeType), WMError::WM_OK);

    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test1";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    ASSERT_EQ(ssm_->SetGlobalDragResizeType(DragResizeType::RESIZE_TYPE_UNDEFINED), WMError::WM_OK);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(ssm_->SetGlobalDragResizeType(dragResizeType), WMError::WM_OK);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: SetGlobalDragResizeType02
 * @tc.desc: test function : SetGlobalDragResizeType invalid session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetGlobalDragResizeType02, TestSize.Level1)
{
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    ASSERT_EQ(ssm_->SetGlobalDragResizeType(dragResizeType), WMError::WM_OK);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test1";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ASSERT_EQ(ssm_->SetGlobalDragResizeType(dragResizeType), WMError::WM_OK);
    usleep(WAIT_SYNC_IN_NS);
    ssm_->sceneSessionMap_.insert({ 0, nullptr });
    ASSERT_EQ(ssm_->SetGlobalDragResizeType(dragResizeType), WMError::WM_OK);
    usleep(WAIT_SYNC_IN_NS);
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert({ 0, nullptr });
    ASSERT_EQ(ssm_->SetGlobalDragResizeType(dragResizeType), WMError::WM_OK);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: GetGlobalDragResizeType
 * @tc.desc: test function : GetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, GetGlobalDragResizeType, TestSize.Level1)
{
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    ASSERT_EQ(ssm_->GetGlobalDragResizeType(dragResizeType), WMError::WM_OK);
}

/**
 * @tc.name: SetAppDragResizeType
 * @tc.desc: test function : SetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetAppDragResizeType, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test1";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    ASSERT_EQ(ssm_->SetAppDragResizeType("", dragResizeType), WMError::WM_ERROR_INVALID_PARAM);
    ASSERT_EQ(ssm_->SetAppDragResizeType(info.bundleName_, dragResizeType), WMError::WM_OK);
}

/**
 * @tc.name: GetDefaultDragResizeType
 * @tc.desc: test function : GetDefaultDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, GetDefaultDragResizeType, TestSize.Level1)
{
    DragResizeType originalDragResizeType = ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_;
    bool originalFreeMultiWindowSupport = ssm_->systemConfig_.freeMultiWindowSupport_;
    ASSERT_EQ(ssm_->SetGlobalDragResizeType(DragResizeType::RESIZE_TYPE_UNDEFINED), WMError::WM_OK);
    // not support
    ssm_->systemConfig_.freeMultiWindowSupport_ = false;
    ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_ = DragResizeType::RESIZE_TYPE_UNDEFINED;
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    ssm_->GetEffectiveDragResizeType(dragResizeType);
    ASSERT_EQ(dragResizeType, DragResizeType::RESIZE_EACH_FRAME);
    // support and default
    ssm_->systemConfig_.freeMultiWindowSupport_ = true;
    ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_ = DragResizeType::RESIZE_TYPE_UNDEFINED;
    dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    ssm_->GetEffectiveDragResizeType(dragResizeType);
    ASSERT_EQ(dragResizeType, DragResizeType::RESIZE_WHEN_DRAG_END);
    // support and set
    ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_ = DragResizeType::RESIZE_WHEN_DRAG_END;
    dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    ssm_->GetEffectiveDragResizeType(dragResizeType);
    ASSERT_EQ(dragResizeType, ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_);

    ssm_->systemConfig_.freeMultiWindowConfig_.defaultDragResizeType_ = originalDragResizeType;
    ssm_->systemConfig_.freeMultiWindowSupport_ = originalFreeMultiWindowSupport;
}

/**
 * @tc.name: GetAppDragResizeType
 * @tc.desc: test function : GetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, GetAppDragResizeType, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test1";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    ASSERT_EQ(ssm_->GetAppDragResizeType(info.bundleName_, dragResizeType), WMError::WM_OK);
}

/**
 * @tc.name: SetAppKeyFramePolicy
 * @tc.desc: test function : SetAppKeyFramePolicy
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetAppKeyFramePolicy, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test1";
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    KeyFramePolicy keyFramePolicy;
    // empty map
    ASSERT_EQ(ssm_->SetAppKeyFramePolicy(info.bundleName_, keyFramePolicy), WMError::WM_OK);
    auto getKeyFramePolicy = ssm_->GetAppKeyFramePolicy(info.bundleName_);
    ASSERT_EQ(getKeyFramePolicy.dragResizeType_, keyFramePolicy.dragResizeType_);
    keyFramePolicy.dragResizeType_ = DragResizeType::RESIZE_KEY_FRAME;
    ASSERT_EQ(ssm_->SetAppKeyFramePolicy(info.bundleName_, keyFramePolicy), WMError::WM_OK);
    getKeyFramePolicy = ssm_->GetAppKeyFramePolicy(info.bundleName_);
    ASSERT_EQ(getKeyFramePolicy.dragResizeType_, keyFramePolicy.dragResizeType_);
    // valid
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ASSERT_EQ(ssm_->SetAppKeyFramePolicy(info.bundleName_, keyFramePolicy), WMError::WM_OK);
    getKeyFramePolicy = ssm_->GetAppKeyFramePolicy(info.bundleName_);
    ASSERT_EQ(getKeyFramePolicy.dragResizeType_, keyFramePolicy.dragResizeType_);
    // nullptr
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), nullptr });
    ASSERT_EQ(ssm_->SetAppKeyFramePolicy(info.bundleName_, keyFramePolicy), WMError::WM_OK);
    getKeyFramePolicy = ssm_->GetAppKeyFramePolicy(info.bundleName_);
    ASSERT_EQ(getKeyFramePolicy.dragResizeType_, keyFramePolicy.dragResizeType_);
    // empty name
    ASSERT_EQ(ssm_->SetAppKeyFramePolicy("", keyFramePolicy), WMError::WM_ERROR_INVALID_PARAM);
    // sub window
    info.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession2);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession2 });
    ASSERT_EQ(ssm_->SetAppKeyFramePolicy(info.bundleName_, keyFramePolicy), WMError::WM_OK);
}

/**
 * @tc.name: BuildCancelPointerEvent
 * @tc.desc: test function : BuildCancelPointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, BuildCancelPointerEvent, TestSize.Level1)
{
    auto pointerEvent = MMI::PointerEvent::Create();
    ssm_->BuildCancelPointerEvent(pointerEvent, 0, MMI::PointerEvent::POINTER_ACTION_DOWN, 2);
    int32_t pointerId = 99999999;
    ASSERT_EQ(pointerEvent->GetId(), pointerId);
    ASSERT_EQ(pointerEvent->GetTargetWindowId(), 2);
    ASSERT_EQ(pointerEvent->GetPointerId(), 0);
    ASSERT_EQ(pointerEvent->GetPointerAction(), MMI::PointerEvent::POINTER_ACTION_CANCEL);
    ASSERT_EQ(pointerEvent->GetSourceType(), MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN);
}

/**
 * @tc.name: MinimizeByWindowId
 * @tc.desc: test function : MinimizeByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, MinimizeByWindowId, TestSize.Level1)
{
    std::vector<int32_t> windowIds;
    WMError res = ssm_->MinimizeByWindowId(windowIds);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, res);
}

/**
 * @tc.name: UpdateAnimationSpeedWithPid
 * @tc.desc: test function : UpdateAnimationSpeedWithPid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, UpdateAnimationSpeedWithPid, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);

    MockAccesstokenKit::MockIsSACalling(false);

    float speed = 2.0f;
    int32_t pid = 1;

    auto result = ssm_->UpdateAnimationSpeedWithPid(pid, speed);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_PERMISSION);

    MockAccesstokenKit::MockIsSACalling(true);
    SessionInfo info;
    info.bundleName_ = "SceneSessionManagerTest7";
    info.abilityName_ = "UpdateAnimationSpeedWithPid";
    sptr<SceneSession> sceneSession01 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession03 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession04 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession01, nullptr);
    ASSERT_NE(sceneSession02, nullptr);
    ASSERT_NE(sceneSession03, nullptr);
    ASSERT_NE(sceneSession04, nullptr);
    ssm_->sceneSessionMap_.insert(std::make_pair(0, nullptr));
    result = ssm_->UpdateAnimationSpeedWithPid(pid, speed);
    EXPECT_EQ(result, WMError::WM_OK);
    sceneSession01->isVisible_ = false;
    sceneSession01->SetCallingPid(2);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession01));
    result = ssm_->UpdateAnimationSpeedWithPid(pid, speed);
    EXPECT_EQ(result, WMError::WM_OK);
    sceneSession02->isVisible_ = true;
    sceneSession02->SetCallingPid(3);
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession02));
    result = ssm_->UpdateAnimationSpeedWithPid(pid, speed);
    EXPECT_EQ(result, WMError::WM_OK);
    sceneSession03->isVisible_ = false;
    sceneSession03->SetCallingPid(pid);
    ssm_->sceneSessionMap_.insert(std::make_pair(3, sceneSession03));
    result = ssm_->UpdateAnimationSpeedWithPid(pid, speed);
    EXPECT_EQ(result, WMError::WM_OK);
    sceneSession04->isVisible_ = true;
    sceneSession04->SetCallingPid(pid);
    ssm_->sceneSessionMap_.insert(std::make_pair(4, sceneSession04));
    result = ssm_->UpdateAnimationSpeedWithPid(pid, speed);
    EXPECT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name: SetForegroundWindowNum
 * @tc.desc: test function : SetForegroundWindowNum
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, SetForegroundWindowNum, TestSize.Level1)
{
    uint32_t windowNum = 0;
    WMError res = ssm_->SetForegroundWindowNum(windowNum);
    if (!ssm_->systemConfig_.freeMultiWindowSupport_) {
        EXPECT_EQ(WMError::WM_ERROR_DEVICE_NOT_SUPPORT, res);
    } else {
        EXPECT_EQ(WMError::WM_OK, res);
        windowNum = 1;
        res = ssm_->SetForegroundWindowNum(windowNum);
        EXPECT_EQ(WMError::WM_OK, res);
    }
}

/**
 * @tc.name: CloneWindow
 * @tc.desc: test function : CloneWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, CloneWindow, TestSize.Level1)
{
    int32_t fromPersistentId = 8;
    int32_t toPersistentId = 11;
    bool needOffScreen = true;
    WSError res = ssm_->CloneWindow(fromPersistentId, toPersistentId, needOffScreen);
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, res);
}

/**
 * @tc.name: ConfigSupportFunctionType
 * @tc.desc: test function : ConfigSupportFunctionType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, ConfigSupportFunctionType, Function | SmallTest | Level3)
{
    ssm_->ConfigSupportFunctionType(SupportFunctionType::ALLOW_KEYBOARD_WILL_ANIMATION_NOTIFICATION);
    EXPECT_EQ(SupportFunctionType::ALLOW_KEYBOARD_WILL_ANIMATION_NOTIFICATION,
        (ssm_->systemConfig_.supportFunctionType_ & SupportFunctionType::ALLOW_KEYBOARD_WILL_ANIMATION_NOTIFICATION));
}
} // namespace
} // namespace Rosen
} // namespace OHOS