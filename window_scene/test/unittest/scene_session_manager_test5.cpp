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
#include <bundle_mgr_proxy.h>
#include <bundlemgr/launcher_service.h>
#include "iremote_object_mocker.h"
#include "interfaces/include/ws_common.h"
#include "iremote_object_mocker.h"
#include "session_manager/include/scene_session_manager.h"
#include "screen_session_manager/include/screen_session_manager.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/main_session.h"
#include "window_manager_agent.h"
#include "session_manager.h"
#include "zidl/window_manager_agent_interface.h"
#include "common_test_utils.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class SceneSessionManagerTest5 : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp() override;

    void TearDown() override;

    static bool gestureNavigationEnabled_;

    static ProcessGestureNavigationEnabledChangeFunc callbackFunc_;
    static sptr<SceneSessionManager> ssm_;
    ScreenSessionManagerClient* screenSessionManagerClient_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerTest5::ssm_ = nullptr;
bool SceneSessionManagerTest5::gestureNavigationEnabled_ = true;

ProcessGestureNavigationEnabledChangeFunc SceneSessionManagerTest5::callbackFunc_ =
    [](bool enable, const std::string& bundleName, GestureBackType type) { gestureNavigationEnabled_ = enable; };

void SceneSessionManagerTest5::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerTest5::TearDownTestCase()
{
    ssm_->sceneSessionMap_.clear();
    ssm_ = nullptr;
}

void SceneSessionManagerTest5::SetUp()
{
    ssm_->sceneSessionMap_.clear();
    screenSessionManagerClient_ = &ScreenSessionManagerClient::GetInstance();
}

void SceneSessionManagerTest5::TearDown()
{
    ssm_->sceneSessionMap_.clear();
    usleep(WAIT_SYNC_IN_NS);
    screenSessionManagerClient_ = nullptr;
}

namespace {
/**
 * @tc.name: NotifySessionTouchOutside
 * @tc.desc: SceneSesionManager notify session touch outside
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, NotifySessionTouchOutside01, TestSize.Level1)
{
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(ssm_, nullptr);
    ssm_->recoveringFinished_ = false;
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, property);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    ssm_->NotifySessionTouchOutside(0, 0);
    property->SetPersistentId(1);
    ssm_->NotifySessionTouchOutside(1, 0);
}

/**
 * @tc.name: SetStartWindowBackgroundColor
 * @tc.desc: SceneSessionManager set start window background color
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, SetStartWindowBackgroundColor, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    auto res = ssm_->SetStartWindowBackgroundColor("mName", "aName", 0xffffffff, 100);
    EXPECT_NE(res, WMError::WM_ERROR_INVALID_CALLING);

    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ssm_->bundleMgr_ = sptr<AppExecFwk::BundleMgrProxy>::MakeSptr(iRemoteObjectMocker);
    res = ssm_->SetStartWindowBackgroundColor("mName", "aName", 0xffffffff, 100);
    EXPECT_NE(res, WMError::WM_ERROR_INVALID_CALLING);
}

/**
 * @tc.name: OnBundleUpdated
 * @tc.desc: Erase cached info when bundle update
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, OnBundleUpdated, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->startingWindowMap_.clear();
    /**
     * @tc.steps: step1. Insert item to map.
     */
    SessionInfo sessionInfo;
    sessionInfo.moduleName_ = "moduleName";
    sessionInfo.abilityName_ = "abilityName";
    sessionInfo.bundleName_ = "bundleName";
    auto key = sessionInfo.moduleName_ + sessionInfo.abilityName_;
    StartingWindowInfo startingWindowInfo;
    std::map<std::string, StartingWindowInfo> startingWindowInfoMap{ { key, startingWindowInfo } };
    ssm_->startingWindowMap_.insert({ sessionInfo.bundleName_, startingWindowInfoMap });
    EXPECT_NE(ssm_->startingWindowMap_.size(), 0);

    /**
     * @tc.steps: step2. On bundle updated and check map.
     */
    ssm_->OnBundleUpdated(sessionInfo.bundleName_, 0);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(ssm_->startingWindowMap_.size(), 0);
}

/**
 * @tc.name: OnConfigurationUpdated
 * @tc.desc: Clear startingWindowMap when configuration update
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, OnConfigurationUpdated, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->startingWindowMap_.clear();
    /**
     * @tc.steps: step1. Insert item to map.
     */
    SessionInfo sessionInfo;
    sessionInfo.moduleName_ = "moduleName";
    sessionInfo.abilityName_ = "abilityName";
    sessionInfo.bundleName_ = "bundleName";
    auto key = sessionInfo.moduleName_ + sessionInfo.abilityName_;
    StartingWindowInfo startingWindowInfo;
    std::map<std::string, StartingWindowInfo> startingWindowInfoMap{ { key, startingWindowInfo } };
    ssm_->startingWindowMap_.insert({ sessionInfo.bundleName_, startingWindowInfoMap });
    EXPECT_NE(ssm_->startingWindowMap_.size(), 0);

    /**
     * @tc.steps: step2. On configuration updated and check map.
     */
    auto configuration = std::make_shared<AppExecFwk::Configuration>();
    ssm_->OnConfigurationUpdated(configuration);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(ssm_->startingWindowMap_.size(), 0);
}

/**
 * @tc.name: PrepareTerminate
 * @tc.desc: SceneSesionManager prepare terminate
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, PrepareTerminate, TestSize.Level1)
{
    int32_t persistentId = 1;
    bool isPrepareTerminate = true;
    SceneSessionManager* sceneSessionManager = sptr<SceneSessionManager>::MakeSptr();
    ASSERT_NE(sceneSessionManager, nullptr);
    EXPECT_EQ(WSError::WS_OK, sceneSessionManager->PrepareTerminate(persistentId, isPrepareTerminate));
}

/**
 * @tc.name: RequestInputMethodCloseKeyboard
 * @tc.desc: RequestInputMethodCloseKeyboard
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestInputMethodCloseKeyboard02, TestSize.Level1)
{
    int32_t persistentId = -1;
    bool isPrepareTerminate = true;
    SceneSessionManager* sceneSessionManager = sptr<SceneSessionManager>::MakeSptr();
    ASSERT_NE(sceneSessionManager, nullptr);
    sceneSessionManager->PrepareTerminate(persistentId, isPrepareTerminate);
}

/**
 * @tc.name: HandleSpecificSystemBarProperty
 * @tc.desc: HandleSpecificSystemBarProperty
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, HandleSpecificSystemBarProperty, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    WindowType type = WindowType::WINDOW_TYPE_STATUS_BAR;
    ssm_->HandleSpecificSystemBarProperty(type, property, sceneSession);
}

/**
 * @tc.name: UpdateBrightness
 * @tc.desc: UpdateBrightness
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, UpdateBrightness, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.isSystem_ = false;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, property);
    ASSERT_NE(property, nullptr);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ssm_->UpdateBrightness(1);
    FocusChangeInfo focusInfo;
    ssm_->GetCurrentUserId();
    ssm_->GetFocusWindowInfo(focusInfo);
}

/**
 * @tc.name: IsNeedUpdateBrightness
 * @tc.desc: IsNeedUpdateBrightness
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, IsNeedUpdateBrightness, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test1";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    info.abilityName_ = "test2";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession1->persistentId_ = 2;
    sceneSession1->state_ = SessionState::STATE_FOREGROUND;
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession1));

    bool isNeed = ssm_->IsNeedUpdateBrightness(1, -1);
    EXPECT_EQ(isNeed, false);
    ssm_->displayBrightness_ = -1;
    isNeed = ssm_->IsNeedUpdateBrightness(1, -1);
    EXPECT_EQ(isNeed, false);
    ssm_->displayBrightness_ = 0;
    ssm_->brightnessSessionId_ = 1;
    isNeed = ssm_->IsNeedUpdateBrightness(1, -1);
    EXPECT_EQ(isNeed, true);
    isNeed = ssm_->IsNeedUpdateBrightness(2, -1);
    EXPECT_EQ(isNeed, false);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: RegisterSessionSnapshotFunc
 * @tc.desc: RegisterSessionSnapshotFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RegisterSessionSnapshotFunc, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->RegisterSessionSnapshotFunc(sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    ssm_->RegisterSessionSnapshotFunc(sceneSession);
    info.isSystem_ = false;
    ssm_->RegisterSessionSnapshotFunc(sceneSession);
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo;
    ssm_->RegisterSessionSnapshotFunc(sceneSession);
}

/**
 * @tc.name: RequestFocusStatus
 * @tc.desc: RequestFocusStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestFocusStatus, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RequestFocusStatus";
    info.bundleName_ = "RequestFocusStatus";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->persistentId_ = 1;
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    WMError ret = ssm_->RequestFocusStatus(1, true, true, FocusChangeReason::DEFAULT);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: RequestFocusStatus01
 * @tc.desc: RequestFocusStatus01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestFocusStatus01, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "RequestFocusStatus01";
    info.bundleName_ = "com.ohos.sceneboard";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->persistentId_ = 1;
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    WMError ret = ssm_->RequestFocusStatus(1, true, true, FocusChangeReason::DEFAULT);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);
    ret = ssm_->RequestFocusStatus(1, false, true, FocusChangeReason::DEFAULT);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_CALLING);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: RequestSessionFocus
 * @tc.desc: RequestSessionFocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestSessionFocus, TestSize.Level0)
{
    FocusChangeReason reason = FocusChangeReason::DEFAULT;
    WSError ret = ssm_->RequestSessionFocus(0, true, reason);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
    ret = ssm_->RequestSessionFocus(100, true, reason);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);

    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetFocusable(false);
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);

    sceneSession1->property_ = property;
    ASSERT_NE(sceneSession1->property_, nullptr);
    sceneSession1->persistentId_ = 1;
    sceneSession1->isVisible_ = true;
    sceneSession1->state_ = SessionState::STATE_ACTIVE;
    sceneSession1->focusedOnShow_ = true;
    sceneSession1->property_->focusable_ = true;
    sceneSession1->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(2);
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });
    ret = ssm_->RequestSessionFocus(1, true, reason);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_EQ(focusGroup->GetFocusedSessionId(), 1);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: SetShiftFocusListener
 * @tc.desc: SetShiftFocusListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, SetShiftFocusListener, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    FocusChangeReason reason = FocusChangeReason::SPLIT_SCREEN;
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->ShiftFocus(DEFAULT_DISPLAY_ID, sceneSession, false, reason);
    info.isSystem_ = true;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    ProcessShiftFocusFunc fun;
    NotifySCBAfterUpdateFocusFunc func;
    ssm_->SetShiftFocusListener(fun);
    ssm_->SetSCBFocusedListener(func);
    ssm_->SetSCBUnfocusedListener(func);
    ProcessStartUIAbilityErrorFunc func2;
    ssm_->SetStartUIAbilityErrorListener(func2);
    ssm_->ShiftFocus(DEFAULT_DISPLAY_ID, sceneSession1, false, reason);
}

/**
 * @tc.name: ShiftFocus
 * @tc.desc: ShiftFocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, ShiftFocus, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "ShiftFocus";
    info.bundleName_ = "ShiftFocus";
    info.isSystem_ = true;
    sptr<SceneSession> focusedSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    focusedSession->persistentId_ = 1;
    sceneSession->persistentId_ = 2;
    focusedSession->GetSessionProperty()->SetDisplayId(0);
    sceneSession->GetSessionProperty()->SetDisplayId(12);
    ssm_->sceneSessionMap_.insert({ focusedSession->GetPersistentId(), focusedSession });
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ssm_->ShiftFocus(DEFAULT_DISPLAY_ID, sceneSession, false, FocusChangeReason::DEFAULT);
}

/**
 * @tc.name: UpdateFocusStatus
 * @tc.desc: UpdateFocusStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, UpdateFocusStatus, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> nextSession = nullptr;
    auto focusNotifyInfo = sptr<FocusNotifyInfo>::MakeSptr();
    ssm_->UpdateFocusStatus(DEFAULT_DISPLAY_ID, nextSession, false, focusNotifyInfo);
    ssm_->UpdateFocusStatus(DEFAULT_DISPLAY_ID, nextSession, true, focusNotifyInfo);

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    ssm_->UpdateFocusStatus(DEFAULT_DISPLAY_ID, sceneSession1, true, focusNotifyInfo);
    ssm_->UpdateFocusStatus(DEFAULT_DISPLAY_ID, sceneSession1, false, focusNotifyInfo);
}

/**
 * @tc.name: UpdateFocusStatus01
 * @tc.desc: UpdateFocusStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, UpdateFocusStatus01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<SceneSession> sceneSession = nullptr;
    auto focusNotifyInfo = sptr<FocusNotifyInfo>::MakeSptr();
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);

    ssm_->UpdateFocusStatus(DEFAULT_DISPLAY_ID, sceneSession, false, focusNotifyInfo);
    ASSERT_EQ(focusGroup->GetFocusedSessionId(), 1);
    ssm_->UpdateFocusStatus(DEFAULT_DISPLAY_ID, sceneSession, true, focusNotifyInfo);
    ASSERT_NE(focusGroup->GetFocusedSessionId(), 1);
}

/**
 * @tc.name: UpdateFocusStatus02
 * @tc.desc: Verify that the NotifyFocusStatusByMission function is called.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, UpdateFocusStatus02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "UpdateFocusStatus02";
    info.bundleName_ = "UpdateFocusStatus02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    auto focusNotifyInfo = sptr<FocusNotifyInfo>::MakeSptr();
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetNeedBlockNotifyFocusStatusUntilForeground(true);
    ssm_->UpdateFocusStatus(DEFAULT_DISPLAY_ID, sceneSession, true, focusNotifyInfo);
    focusNotifyInfo = nullptr;
    ssm_->UpdateFocusStatus(DEFAULT_DISPLAY_ID, sceneSession, true, focusNotifyInfo);
}

/**
 * @tc.name: RequestSessionUnfocus
 * @tc.desc: RequestSessionUnfocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestSessionUnfocus, TestSize.Level0)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    FocusChangeReason reason = FocusChangeReason::MOVE_UP;
    WSError ret = ssm_->RequestSessionUnfocus(0, reason);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);

    sptr<SceneSession> focusedSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(focusedSession, nullptr);
    focusedSession->property_ = property;
    ASSERT_NE(focusedSession->property_, nullptr);
    sceneSession1->persistentId_ = 1;
    focusedSession->persistentId_ = 2;
    focusedSession->property_->parentPersistentId_ = 1;
    ssm_->windowFocusController_->UpdateFocusedSessionId(DEFAULT_DISPLAY_ID, 1);
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });
    ssm_->sceneSessionMap_.insert({ focusedSession->GetPersistentId(), focusedSession });
    ret = ssm_->RequestSessionUnfocus(1, reason);
    ASSERT_EQ(ret, WSError::WS_OK);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    ASSERT_NE(focusGroup->GetFocusedSessionId(), 1);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: RequestFocusSpecificCheck
 * @tc.desc: RequestFocusSpecificCheck
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestFocusSpecificCheck, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    FocusChangeReason reason = FocusChangeReason::MOVE_UP;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    WSError ret = ssm_->RequestFocusSpecificCheck(DEFAULT_DISPLAY_ID, sceneSession1, true, reason);
    EXPECT_EQ(ret, WSError::WS_OK);
    FocusChangeReason reason1 = FocusChangeReason::SPLIT_SCREEN;
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    ret = ssm_->RequestFocusSpecificCheck(DEFAULT_DISPLAY_ID, sceneSession1, true, reason1);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: NotifyFocusStatus
 * @tc.desc: NotifyFocusStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, NotifyFocusStatus, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    auto focusGroup = sptr<FocusGroup>::MakeSptr(DEFAULT_DISPLAY_ID);
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    auto focusNotifyInfo = sptr<FocusNotifyInfo>::MakeSptr();
    ssm_->NotifyFocusStatus(sceneSession1, false, focusGroup, focusNotifyInfo);
    info.isSystem_ = true;
    ssm_->NotifyFocusStatus(sceneSession1, true, focusGroup, focusNotifyInfo);
    focusNotifyInfo = nullptr;
    ssm_->NotifyFocusStatus(sceneSession1, true, focusGroup, focusNotifyInfo);
}

/**
 * @tc.name: NotifyFocusStatusByMission
 * @tc.desc: NotifyFocusStatusByMission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, NotifyFocusStatusByMission, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<SceneSession> currSession = nullptr;
    ssm_->NotifyFocusStatusByMission(sceneSession, currSession);
    ASSERT_EQ(false, ssm_->MissionChanged(sceneSession, currSession));
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->NotifyFocusStatusByMission(sceneSession, currSession);
    ssm_->MissionChanged(sceneSession, currSession);
    currSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(currSession, nullptr);
    ssm_->NotifyFocusStatusByMission(sceneSession, currSession);
    ssm_->MissionChanged(sceneSession, currSession);
    info.isSystem_ = true;
    ssm_->NotifyFocusStatusByMission(sceneSession, currSession);
    ssm_->MissionChanged(sceneSession, currSession);
}

/**
 * @tc.name: RequestSessionFocusImmediately
 * @tc.desc: RequestSessionFocusImmediately
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestSessionFocusImmediately, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    WSError ret = ssm_->RequestSessionFocusImmediately(0);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
    auto focusGroup = sptr<FocusGroup>::MakeSptr(DEFAULT_DISPLAY_ID);
    ssm_->RequestFocusBasicCheck(0, focusGroup);
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    ret = ssm_->RequestSessionFocusImmediately(1);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
    ssm_->RequestFocusBasicCheck(1, focusGroup);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: RequestFocusBasicCheck
 * @tc.desc: RequestFocusBasicCheck
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestFocusBasicCheck, TestSize.Level1)
{
    sptr<FocusGroup> focusGroup = nullptr;
    WSError ret = ssm_->RequestFocusBasicCheck(0, focusGroup);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
    ret = ssm_->RequestFocusBasicCheck(1, focusGroup);
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);
    focusGroup = sptr<FocusGroup>::MakeSptr(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    ret = ssm_->RequestFocusBasicCheck(1, focusGroup);
    ASSERT_EQ(ret, WSError::WS_DO_NOTHING);
    ret = ssm_->RequestFocusBasicCheck(2, focusGroup);
    ASSERT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: CheckFocusIsDownThroughBlockingType
 * @tc.desc: CheckFocusIsDownThroughBlockingType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CheckFocusIsDownThroughBlockingType, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test1";
    SessionInfo info2;
    info2.abilityName_ = "test2";
    info2.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    bool ret = ssm_->CheckFocusIsDownThroughBlockingType(sceneSession1, sceneSession2, true);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: CheckFocusIsDownThroughBlockingType
 * @tc.desc: CheckFocusIsDownThroughBlockingType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CheckFocusIsDownThroughBlockingType01, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> requestSceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(requestSceneSession, nullptr);
    sptr<SceneSession> focusedSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(focusedSession, nullptr);
    bool includingAppSession = true;
    bool ret = ssm_->CheckFocusIsDownThroughBlockingType(requestSceneSession, focusedSession, includingAppSession);
    ASSERT_EQ(ret, false);

    requestSceneSession->SetZOrder(0);
    focusedSession->SetZOrder(1);
    focusedSession->blockingFocus_ = true;
    focusedSession->state_ = SessionState::STATE_FOREGROUND;
    focusedSession->isVisible_ = true;
    ssm_->sceneSessionMap_.insert({ 0, requestSceneSession });
    ssm_->sceneSessionMap_.insert({ 1, focusedSession });
    ret = ssm_->CheckFocusIsDownThroughBlockingType(requestSceneSession, focusedSession, includingAppSession);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: CheckTopmostWindowFocus
 * @tc.desc: CheckTopmostWindowFocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CheckTopmostWindowFocus, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";

    sptr<SceneSession> focusedSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(focusedSession, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    bool ret = ssm_->CheckTopmostWindowFocus(focusedSession, sceneSession);
    ASSERT_EQ(ret, false);

    focusedSession = sptr<MainSession>::MakeSptr(info, nullptr);
    focusedSession->SetTopmost(true);
    ret = ssm_->CheckTopmostWindowFocus(focusedSession, sceneSession);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: CheckRequestFocusImmediately01
 * @tc.desc: CheckRequestFocusImmediately01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CheckRequestFocusImmediately01, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    bool ret = ssm_->CheckRequestFocusImmediately(sceneSession);
    ASSERT_EQ(ret, false);

    sptr<Session> session = sptr<Session>::MakeSptr(info);
    session->persistentId_ = 1;
    sceneSession->dialogVec_.push_back(session);
    ssm_->windowFocusController_->UpdateFocusedSessionId(DEFAULT_DISPLAY_ID, 1);
    ret = ssm_->CheckRequestFocusImmediately(sceneSession);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: CheckRequestFocusImmediately02
 * @tc.desc: CheckRequestFocusImmediately02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CheckRequestFocusImmediately02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CheckRequestFocusImmediately02";
    sessionInfo.abilityName_ = "CheckRequestFocusImmediately02";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::APP_SUB_WINDOW_BASE);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    EXPECT_EQ(WindowType::APP_SUB_WINDOW_BASE, sceneSession->GetWindowType());
    bool ret = ssm_->CheckRequestFocusImmediately(sceneSession);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: CheckRequestFocusSubWindowImmediately01
 * @tc.desc: CheckRequestFocusSubWindowImmediately01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CheckRequestFocusSubWindowImmediately01, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CheckRequestFocusSubWindowImmediately01";
    sessionInfo.abilityName_ = "CheckRequestFocusSubWindowImmediately01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    bool ret = ssm_->CheckRequestFocusSubWindowImmediately(sceneSession);
    EXPECT_EQ(ret, false);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ret = ssm_->CheckRequestFocusSubWindowImmediately(sceneSession);
    EXPECT_EQ(ret, false);

    SessionInfo subSessionInfo;
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(subSessionInfo, nullptr);
    subSession->SetSessionState(SessionState::STATE_FOREGROUND);
    subSession->persistentId_ = 2;
    subSession->SetFocusable(true);
    subSession->SetFocusedOnShow(true);
    sceneSession->subSession_.push_back(subSession);

    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, subSession));
    ssm_->windowFocusController_->UpdateFocusedSessionId(DEFAULT_DISPLAY_ID, 2);
    ret = ssm_->CheckRequestFocusSubWindowImmediately(sceneSession);
    EXPECT_EQ(ret, true);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: CheckRequestFocusSubWindowImmediately02
 * @tc.desc: CheckRequestFocusSubWindowImmediately02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CheckRequestFocusSubWindowImmediately02, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "CheckRequestFocusSubWindowImmediately02";
    sessionInfo.abilityName_ = "CheckRequestFocusSubWindowImmediately02";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    bool ret = ssm_->CheckRequestFocusSubWindowImmediately(sceneSession);
    EXPECT_EQ(ret, false);

    SessionInfo subSessionInfo;
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(subSessionInfo, nullptr);
    subSession->SetSessionState(SessionState::STATE_FOREGROUND);
    subSession->persistentId_ = 2;
    subSession->SetFocusable(true);
    subSession->SetFocusedOnShow(true);
    sceneSession->subSession_.push_back(subSession);

    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, subSession));
    ssm_->windowFocusController_->UpdateFocusedSessionId(DEFAULT_DISPLAY_ID, 2);
    ret = ssm_->CheckRequestFocusSubWindowImmediately(sceneSession);
    EXPECT_EQ(ret, true);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: GetNextFocusableSession
 * @tc.desc: GetNextFocusableSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, GetNextFocusableSession, TestSize.Level1)
{
    int32_t persistentId = 0;
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<SceneSession> result = ssm_->GetNextFocusableSession(DEFAULT_DISPLAY_ID, persistentId);
    ASSERT_EQ(result, nullptr);

    sceneSession->SetForceHideState(ForceHideState::HIDDEN_WHEN_FOCUSED);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    persistentId = 1;
    result = ssm_->GetNextFocusableSession(DEFAULT_DISPLAY_ID, persistentId);
    ASSERT_EQ(result, nullptr);
}

/**
 * @tc.name: GetNextFocusableSession_Desktop
 * @tc.desc: Get desktop with extend screen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, GetNextFocusableSession_Desktop, TestSize.Level1)
{
    int32_t persistentId = 0;
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();

    SessionInfo info1;
    info1.abilityName_ = "testWindow1";
    info1.bundleName_ = "testWindow1";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    sceneSession1->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession1->persistentId_ = 1;
    sceneSession1->SetZOrder(120);

    SessionInfo info2;
    info2.abilityName_ = "testWindow2";
    info2.bundleName_ = "testWindow2";
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    sceneSession2->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession2->persistentId_ = 2;
    sceneSession2->SetZOrder(130);

    SessionInfo info3;
    info3.abilityName_ = "desktop1";
    info3.bundleName_ = "desktop1";
    sptr<SceneSession> desktopSession1 = sptr<SceneSession>::MakeSptr(info3, nullptr);
    desktopSession1->property_->SetWindowType(WindowType::WINDOW_TYPE_DESKTOP);
    desktopSession1->persistentId_ = 3;
    desktopSession1->UpdateVisibilityInner(true);
    desktopSession1->SetSessionState(SessionState::STATE_FOREGROUND);
    desktopSession1->SetZOrder(1);
    desktopSession1->property_->SetDisplayId(0);

    SessionInfo info4;
    info4.abilityName_ = "desktop2";
    info4.bundleName_ = "desktop2";
    sptr<SceneSession> desktopSession2 = sptr<SceneSession>::MakeSptr(info4, nullptr);
    desktopSession2->property_->SetWindowType(WindowType::WINDOW_TYPE_DESKTOP);
    desktopSession2->persistentId_ = 4;
    desktopSession2->UpdateVisibilityInner(true);
    desktopSession2->SetSessionState(SessionState::STATE_FOREGROUND);
    desktopSession2->SetZOrder(1);
    desktopSession2->property_->SetDisplayId(11);

    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession1));
    ssm_->sceneSessionMap_.insert(std::make_pair(2, sceneSession2));
    ssm_->sceneSessionMap_.insert(std::make_pair(3, desktopSession1));
    ssm_->sceneSessionMap_.insert(std::make_pair(4, desktopSession2));

    sptr<SceneSession> result = ssm_->GetNextFocusableSession(0, 1);
    EXPECT_EQ(result, nullptr);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: GetTopNearestBlockingFocusSession
 * @tc.desc: GetTopNearestBlockingFocusSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, GetTopNearestBlockingFocusSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";

    ssm_->GetTopNearestBlockingFocusSession(DEFAULT_DISPLAY_ID, 2, true);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->GetTopNearestBlockingFocusSession(DEFAULT_DISPLAY_ID, 0, true);

    Session session = Session(info);
    session.property_ = nullptr;
    ssm_->GetTopNearestBlockingFocusSession(DEFAULT_DISPLAY_ID, 0, true);
    sptr<SceneSession> session_ = nullptr;
    ssm_->GetTopNearestBlockingFocusSession(DEFAULT_DISPLAY_ID, 0, true);
    session_ = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(session_, nullptr);
    ssm_->GetTopNearestBlockingFocusSession(DEFAULT_DISPLAY_ID, 0, true);
}

/**
 * @tc.name: NotifyMMIWindowPidChange
 * @tc.desc: NotifyMMIWindowPidChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, NotifyMMIWindowPidChange, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->NotifyMMIWindowPidChange(0, true);
    sceneSession = ssm_->CreateSceneSession(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
}

/**
 * @tc.name: InitSnapshotCache
 * @tc.desc: InitSnapshotCache
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, InitSnapshotCache, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->InitSnapshotCache();
    if (ssm_->systemConfig_.windowUIType_ == WindowUIType::PC_WINDOW) {
        ASSERT_EQ(ssm_->snapshotCapacity_, 50);
    } else if (ssm_->systemConfig_.windowUIType_ == WindowUIType::PAD_WINDOW) {
        ASSERT_EQ(ssm_->snapshotCapacity_, 0);
    } else {
        ASSERT_EQ(ssm_->snapshotCapacity_, 0);
    }
}

/**
 * @tc.name: PutSnapshotToCache
 * @tc.desc: PutSnapshotToCache
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, PutSnapshotToCache, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.persistentId_ = 30;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    std::string bundleName = "testBundleName";
    int32_t persistentId = 30;
    sceneSession->scenePersistence_ = sptr<ScenePersistence>::MakeSptr(bundleName, persistentId);
    sceneSession->snapshot_ = std::make_shared<Media::PixelMap>();
    ssm_->sceneSessionMap_.insert({ 30, sceneSession });
    for (int32_t id = 30; id <= 30 + ssm_->snapshotCapacity_; ++id) {
        ssm_->PutSnapshotToCache(id);
    }
    ASSERT_EQ(sceneSession->snapshot_, nullptr);
}

/**
 * @tc.name: VisitSnapshotFromCache
 * @tc.desc: VisitSnapshotFromCache
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, VisitSnapshotFromCache, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.persistentId_ = 30;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    std::string bundleName = "testBundleName";
    int32_t persistentId = 30;
    sceneSession->scenePersistence_ = sptr<ScenePersistence>::MakeSptr(bundleName, persistentId);
    ssm_->sceneSessionMap_.insert({ 30, sceneSession });
    sceneSession->snapshot_ = std::make_shared<Media::PixelMap>();
    for (int32_t id = 30; id < 30 + ssm_->snapshotCapacity_; ++id) {
        ssm_->PutSnapshotToCache(id);
    }
    ssm_->PutSnapshotToCache(30 + ssm_->snapshotCapacity_);
    ssm_->VisitSnapshotFromCache(30);
    ASSERT_EQ(sceneSession->snapshot_, nullptr);
}

/**
 * @tc.name: RemoveSnapshotFromCache
 * @tc.desc: RemoveSnapshotFromCache
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RemoveSnapshotFromCache, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.persistentId_ = 30;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    std::string bundleName = "testBundleName";
    int32_t persistentId = 30;
    sceneSession->scenePersistence_ = sptr<ScenePersistence>::MakeSptr(bundleName, persistentId);
    ssm_->sceneSessionMap_.insert({ 30, sceneSession });
    sceneSession->snapshot_ = std::make_shared<Media::PixelMap>();
    for (int32_t id = 30; id < 30 + ssm_->snapshotCapacity_; ++id) {
        ssm_->PutSnapshotToCache(id);
    }
    ssm_->RemoveSnapshotFromCache(30);
    ASSERT_EQ(sceneSession->snapshot_, nullptr);
}

/**
 * @tc.name: RequestSceneSessionBackground
 * @tc.desc: RequestSceneSessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestSceneSessionBackground, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.ancoSceneState = 0;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<SceneSession> sceneSession = nullptr;
    std::shared_ptr<std::promise<int32_t>> promise = std::make_shared<std::promise<int32_t>>();
    ASSERT_EQ(WSError::WS_OK, ssm_->RequestSceneSessionBackground(sceneSession, true, true, true));
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_EQ(WSError::WS_OK, ssm_->RequestSceneSessionBackground(sceneSession, false, false, false));
    ASSERT_EQ(WSError::WS_OK, ssm_->RequestSceneSessionBackground(sceneSession, true, false, true));
    ASSERT_EQ(WSError::WS_OK, ssm_->RequestSceneSessionBackground(sceneSession, true, true, true));
}

/**
 * @tc.name: RequestSceneSessionBackground
 * @tc.desc: RequestSceneSessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestSceneSessionBackground02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.ancoSceneState = 0;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    ASSERT_NE(session, nullptr);
    sptr<SceneSession> sceneSession = nullptr;
    std::shared_ptr<std::promise<int32_t>> promise = std::make_shared<std::promise<int32_t>>();
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->SetSessionInfoPersistentId(123);
    ASSERT_EQ(WSError::WS_OK, ssm_->RequestSceneSessionBackground(sceneSession, false, false, false));
    session->SetSessionInfoPersistentId(0);
    ASSERT_EQ(WSError::WS_OK, ssm_->RequestSceneSessionBackground(sceneSession, false, false, true));
}

/**
 * @tc.name: ConfigAppWindowShadow
 * @tc.desc: SceneSesionManager config app window shadow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, ConfigAppWindowShadow03, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    WindowSceneConfig::ConfigItem item;
    WindowSceneConfig::ConfigItem shadowConfig;
    WindowShadowConfig outShadow;
    std::vector<float> floatTest = { 0.0f, 0.1f, 0.2f, 0.3f };
    bool result = ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);
    ASSERT_EQ(result, true);

    item.SetValue(floatTest);
    shadowConfig.SetValue({ { "offsetX", item } });
    ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);

    shadowConfig.SetValue({ { "offsetY", item } });
    ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);

    item.SetValue(new std::string(""));
    shadowConfig.SetValue({ { "color", item } });
    ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);
}

/**
 * @tc.name: CreateAndConnectSpecificSession_forToastSubWindow
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CreateAndConnectSpecificSession_forToastSubWindow, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "CreateAndConnectSpecificSession_forToastSubWindow";
    info.bundleName_ = "CreateAndConnectSpecificSession_forToastSubWindow";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->property_->SetSubWindowLevel(MAX_SUB_WINDOW_LEVEL);
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<ISession> session;
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token;
    int32_t id = 0;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    property->SetParentPersistentId(sceneSession->GetPersistentId());
    property->SetParentId(sceneSession->GetPersistentId());

    auto res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_WINDOW, res);

    property->AddWindowFlag(WindowFlag::WINDOW_FLAG_IS_TOAST);
    res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    EXPECT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: CreateAndConnectSpecificSession
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CreateAndConnectSpecificSession02, TestSize.Level0)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(sessionStage, nullptr);
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    ASSERT_NE(eventChannel, nullptr);
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<ISession> session;
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token;
    int32_t id = 0;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property->SetWindowFlags(123);
    WSError res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, res); // create main window, property must be nullptr

    sessionStage = sptr<SessionStageMocker>::MakeSptr();
    property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    property->SetWindowFlags(123);
    res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(WSError::WS_OK, res);

    sessionStage = sptr<SessionStageMocker>::MakeSptr();
    property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    property->SetWindowFlags(123);
    res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(WSError::WS_ERROR_NOT_SYSTEM_APP, res);

    property->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    property->SetFloatingWindowAppType(true);
    ssm_->shouldHideNonSecureFloatingWindows_.store(true);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    res = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, id, session, systemConfig, token);
    ASSERT_EQ(WSError::WS_ERROR_NOT_SYSTEM_APP, res);
    ssm_->shouldHideNonSecureFloatingWindows_.store(false);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;
}

/**
 * @tc.name: ProcessDialogRequestFocusImmediately
 * @tc.desc: ProcessDialogRequestFocusImmediately
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, ProcessDialogRequestFocusImmediately02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.persistentId_ = 123;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    auto ret = ssm_->ProcessDialogRequestFocusImmediately(sceneSession);
    EXPECT_EQ(WSError::WS_DO_NOTHING, ret);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: CheckFloatWindowIsAnco01
 * @tc.desc: CheckFloatWindowIsAnco
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CheckFloatWindowIsAnco01, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.moduleName_ = "test2";
    info.persistentId_ = 123;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetCallingPid(123);
    ssm_->sceneSessionMap_.insert({ 123, sceneSession });
    ssm_->CheckFloatWindowIsAnco(123, sceneSession1);
    ASSERT_EQ(sceneSession1->GetIsAncoForFloatingWindow(), false);
    ssm_->sceneSessionMap_.erase(123);
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: RequestSceneSessionByCall
 * @tc.desc: SceneSesionManager request scene session by call
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestSceneSessionByCall02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->RequestSceneSessionByCall(nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->RequestSceneSessionByCall(sceneSession);
}

/**
 * @tc.name: GetAllAbilityInfos
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, GetAllAbilityInfos02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    AAFwk::Want want;
    AppExecFwk::ElementName elementName = want.GetElement();
    int32_t userId = 1;
    std::vector<SCBAbilityInfo> scbAbilityInfos;
    ssm_->GetAllAbilityInfos(want, userId, scbAbilityInfos);

    elementName.bundleName_ = "test";
    ssm_->GetAllAbilityInfos(want, userId, scbAbilityInfos);

    elementName.abilityName_ = "test";
    ssm_->GetAllAbilityInfos(want, userId, scbAbilityInfos);

    elementName.bundleName_ = "";
    ssm_->GetAllAbilityInfos(want, userId, scbAbilityInfos);
}

/**
 * @tc.name: FindMainWindowWithToken
 * @tc.desc: SceneSesionManager find main window with token
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, FindMainWindowWithToken02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.persistentId_ = 123;
    sptr<IRemoteObject> targetToken = nullptr;
    ssm_->FindMainWindowWithToken(targetToken);
    targetToken = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(targetToken, nullptr);
    ssm_->FindMainWindowWithToken(targetToken);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->FindMainWindowWithToken(targetToken);
}

/**
 * @tc.name: FindSessionByToken
 * @tc.desc: SceneSesionManager FindSessionByToken
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, FindSessionByToken, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.persistentId_ = 123;
    sptr<IRemoteObject> targetToken = nullptr;
    auto result = ssm_->FindSessionByToken(targetToken);
    ASSERT_EQ(result, nullptr);

    targetToken = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_NE(targetToken, nullptr);
    result = ssm_->FindSessionByToken(targetToken);
    ASSERT_EQ(result, nullptr);

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->abilityToken_ = targetToken;
    ssm_->sceneSessionMap_.insert({ 123, sceneSession });
    result = ssm_->FindSessionByToken(targetToken, sceneSession->GetWindowType());
    ASSERT_EQ(result, sceneSession);
}

/**
 * @tc.name: RequestSceneSessionBackground
 * @tc.desc: RequestSceneSessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestSceneSessionBackground03, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.persistentId_ = 0;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    ASSERT_NE(session, nullptr);
    std::shared_ptr<std::promise<int32_t>> promise = std::make_shared<std::promise<int32_t>>();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->SetSessionInfoPersistentId(0);
    ssm_->RequestSceneSessionBackground(sceneSession, false, false, true);

    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert({ 0, sceneSession });
    ssm_->RequestSceneSessionBackground(sceneSession, false, false, true);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: ConfigDecor
 * @tc.desc: SceneSesionManager config decor
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, ConfigDecor02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    WindowSceneConfig::ConfigItem* item = new WindowSceneConfig::ConfigItem;
    ASSERT_NE(item, nullptr);
    ssm_->ConfigDecor(*item, false);
}

/**
 * @tc.name: SetSkipSelfWhenShowOnVirtualScreen
 * @tc.desc: SetSkipSelfWhenShowOnVirtualScreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, SetSkipSelfWhenShowOnVirtualScreen, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(property, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    uint64_t surfaceNodeId = 1234;
    ssm_->SetSkipSelfWhenShowOnVirtualScreen(surfaceNodeId, false);
    ssm_->SetSkipSelfWhenShowOnVirtualScreen(surfaceNodeId, true);
}

/**
 * @tc.name: InitSceneSession02
 * @tc.desc: InitSceneSession02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, InitSceneSession02, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "test1";
    sessionInfo.abilityName_ = "test2";
    sessionInfo.abilityInfo = nullptr;
    sessionInfo.isAtomicService_ = true;
    sessionInfo.screenId_ = SCREEN_ID_INVALID;
    unsigned int flags = 1111;
    sessionInfo.want = std::make_shared<AAFwk::Want>();
    ASSERT_NE(nullptr, sessionInfo.want);
    sessionInfo.want->SetFlags(flags);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));

    ssm_->InitSceneSession(sceneSession, sessionInfo, nullptr);
    ASSERT_EQ(DISPLAY_ID_INVALID, sceneSession->GetSessionInfo().screenId_);
}

/**
 * @tc.name: PrepareTerminate
 * @tc.desc: SceneSesionManager prepare terminate
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, PrepareTerminate03, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "PrepareTerminate";
    sessionInfo.abilityName_ = "PrepareTerminate";
    sessionInfo.abilityInfo = nullptr;
    int32_t persistentId = 1;
    bool isPrepareTerminate = true;
    ASSERT_EQ(WSError::WS_OK, ssm_->PrepareTerminate(persistentId, isPrepareTerminate));
}

/**
 * @tc.name: AddClientDeathRecipient
 * @tc.desc: SceneSesionManager add client death recipient
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, AddClientDeathRecipient02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "AddClientDeathRecipient";
    info.bundleName_ = "AddClientDeathRecipient";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->AddClientDeathRecipient(nullptr, sceneSession);
    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    ssm_->AddClientDeathRecipient(sessionStage, sceneSession);
}

/**
 * @tc.name: RequestFocusStatusBySCB
 * @tc.desc: SceneSessionManager request focus status from SCB
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestFocusStatusBySCB, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "RequestFocusTest";
    info.bundleName_ = "RequestFocusTest";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetFocusable(true);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->property_ = property;
    sceneSession->persistentId_ = 1;
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    FocusChangeReason reason = FocusChangeReason::FOREGROUND;
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    ssm_->RequestFocusStatusBySCB(1, true, false, reason);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(focusGroup->GetFocusedSessionId(), 1);

    reason = FocusChangeReason::CLICK;
    ssm_->RequestFocusStatusBySCB(1, true, false, reason);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(focusGroup->GetFocusedSessionId(), 1);
}

/**
 * @tc.name: RequestFocusStatusBySCB01
 * @tc.desc: SceneSessionManager request focus status from SCB
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestFocusStatusBySCB01, TestSize.Level1)
{
    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);

    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "RequestFocusTest";
    info.bundleName_ = "RequestFocusTest";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetFocusable(true);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->property_ = property;
    sceneSession->persistentId_ = 1;
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession1->property_->SetFocusable(true);
    sceneSession1->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession1->persistentId_ = 2;
    sceneSession1->isVisible_ = true;
    sceneSession1->state_ = SessionState::STATE_ACTIVE;
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });
    FocusChangeReason reason = FocusChangeReason::FOREGROUND;
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    ssm_->RequestFocusStatusBySCB(2, true, false, reason);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(focusGroup->GetFocusedSessionId(), 2);

    ssm_->RequestFocusStatusBySCB(1, false, false, reason);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_NE(focusGroup->GetFocusedSessionId(), 1);

    reason = FocusChangeReason::MOVE_UP;
    ssm_->RequestFocusStatusBySCB(1, true, false, reason);
    usleep(WAIT_SYNC_IN_NS);
    FocusChangeReason reasonResult = sceneSession->GetPostProcessFocusState().reason_;
    ASSERT_EQ(reasonResult, FocusChangeReason::MOVE_UP);

    reason = FocusChangeReason::DEFAULT;
    ssm_->RequestFocusStatusBySCB(1, true, false, reason);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(focusGroup->GetFocusedSessionId(), 1);
    screenSessionManagerClient_->screenSessionMap_.clear();
}

/**
 * @tc.name: RequestSessionUnfocus01
 * @tc.desc: RequestSessionUnfocus01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestSessionUnfocus01, TestSize.Level1)
{
    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);

    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "RequestSessionUnfocus01";
    info.bundleName_ = "RequestSessionUnfocus01";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->zOrder_ = 1;
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_DISCONNECT;
    sceneSession->SetFocusable(true);
    sceneSession->property_->SetDisplayId(DEFAULT_DISPLAY_ID);

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession1->persistentId_ = 2;
    sceneSession1->zOrder_ = 2;
    sceneSession1->property_->SetDisplayId(DEFAULT_DISPLAY_ID);
    sceneSession1->isVisible_ = true;
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });
    ssm_->SetFocusedSessionId(2, DEFAULT_DISPLAY_ID);
    ssm_->RequestSessionUnfocus(2, FocusChangeReason::DEFAULT);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    ASSERT_EQ(focusGroup->GetFocusedSessionId(), 1);
    ASSERT_EQ(focusGroup->GetNeedBlockNotifyFocusStatusUntilForeground(), true);
    screenSessionManagerClient_->screenSessionMap_.clear();
}

/**
 * @tc.name: RequestSessionUnfocus02
 * @tc.desc: RequestSessionUnfocus02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestSessionUnfocus02, TestSize.Level1)
{
    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);

    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "RequestSessionUnfocus02";
    info.bundleName_ = "RequestSessionUnfocus02";

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->zOrder_ = 1;
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->SetFocusable(true);
    sceneSession->property_->SetDisplayId(DEFAULT_DISPLAY_ID);

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession1->persistentId_ = 2;
    sceneSession1->zOrder_ = 2;
    sceneSession1->property_->SetDisplayId(DEFAULT_DISPLAY_ID);
    sceneSession1->isVisible_ = true;
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });
    ssm_->SetFocusedSessionId(2, DEFAULT_DISPLAY_ID);
    ssm_->RequestSessionUnfocus(2, FocusChangeReason::DEFAULT);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    ASSERT_EQ(focusGroup->GetFocusedSessionId(), 1);
    ASSERT_EQ(focusGroup->GetNeedBlockNotifyFocusStatusUntilForeground(), false);
    screenSessionManagerClient_->screenSessionMap_.clear();
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: RegisterSaveSnapshotFunc
 * @tc.desc: RegisterSaveSnapshotFunc Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RegisterSaveSnapshotFunc, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<SceneSession> sceneSession = nullptr;
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, ssm_->RegisterSaveSnapshotFunc(sceneSession));

    SessionInfo info;
    info.windowType_ = 1000;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->property_->SetPersistentId(1);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_WINDOW, ssm_->RegisterSaveSnapshotFunc(sceneSession));

    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ASSERT_EQ(WSError::WS_OK, ssm_->RegisterSaveSnapshotFunc(sceneSession));
    usleep(WAIT_SYNC_IN_NS);
}

/**
 * @tc.name: GetDelayRemoveSnapshot
 * @tc.desc: GetDelayRemoveSnapshot Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, GetDelayRemoveSnapshot, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    auto res = ssm_->GetDelayRemoveSnapshot();
    ASSERT_EQ(false, res);
}

/**
 * @tc.name: SetDelayRemoveSnapshot
 * @tc.desc: SetDelayRemoveSnapshot Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, SetDelayRemoveSnapshot, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->SetDelayRemoveSnapshot(true);
    auto res = ssm_->GetDelayRemoveSnapshot();
    ASSERT_EQ(true, res);
}

/**
 * @tc.name: GetTopFloatingSession
 * @tc.desc: GetTopFloatingSession Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, GetTopFloatingSession, TestSize.Level3)
{
    ssm_->sceneSessionMap_.clear();
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "GetTopFloatingSession";
    info.bundleName_ = "GetTopFloatingSession";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->zOrder_ = 1;
    sceneSession->property_->SetDisplayId(DEFAULT_DISPLAY_ID);
    ssm_->SetFocusedSessionId(1, DEFAULT_DISPLAY_ID);

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession1->persistentId_ = 2;
    sceneSession1->zOrder_ = 2;
    sceneSession1->property_->SetDisplayId(DEFAULT_DISPLAY_ID);
    sceneSession1->SetFocusable(true);
    sceneSession1->isVisible_ = true;
    sptr<SceneSession> result = ssm_->GetTopFloatingSession(DEFAULT_DISPLAY_ID, 1);
    EXPECT_EQ(result, nullptr);
    sceneSession1->property_->windowMode_ = WindowMode::WINDOW_MODE_FLOATING;
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession->GetPersistentId(), sceneSession));
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession1->GetPersistentId(), sceneSession1));
    result = ssm_->GetTopFloatingSession(DEFAULT_DISPLAY_ID, 1);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->GetPersistentId(), sceneSession1->GetPersistentId());
}

/**
 * @tc.name: GetNextFocusableSessionWhenFloatWindowExist
 * @tc.desc: GetNextFocusableSessionWhenFloatWindowExist Test
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, GetNextFocusableSessionWhenFloatWindowExist, TestSize.Level3)
{
    ASSERT_NE(screenSessionManagerClient_, nullptr);
    screenSessionManagerClient_->screenSessionMap_.clear();
    ScreenId screenId = 0;
    sptr<ScreenSession> screenSession = new ScreenSession(screenId, ScreenProperty(), 0);
    screenSessionManagerClient_->screenSessionMap_.emplace(screenId, screenSession);

    ssm_->sceneSessionMap_.clear();
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "GetNextFocusableSessionWhenFloatWindowExist";
    info.bundleName_ = "GetNextFocusableSessionWhenFloatWindowExist";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->persistentId_ = 1;
    sceneSession->zOrder_ = 1;
    sceneSession->property_->SetDisplayId(DEFAULT_DISPLAY_ID);

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession->GetPersistentId(), sceneSession));
    sptr<SceneSession> result =
        ssm_->GetNextFocusableSessionWhenFloatWindowExist(DEFAULT_DISPLAY_ID, sceneSession->GetPersistentId());
    EXPECT_EQ(result, nullptr);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PAD_WINDOW;
    ssm_->systemConfig_.freeMultiWindowEnable_ = true;
    ssm_->systemConfig_.freeMultiWindowSupport_ = true;
    result = ssm_->GetNextFocusableSessionWhenFloatWindowExist(DEFAULT_DISPLAY_ID, sceneSession->GetPersistentId());
    EXPECT_EQ(result, nullptr);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(result, nullptr);

    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession1->persistentId_ = 2;
    sceneSession1->zOrder_ = 2;
    sceneSession1->property_->SetDisplayId(DEFAULT_DISPLAY_ID);
    sceneSession1->SetFocusable(true);
    sceneSession1->isVisible_ = true;
    sceneSession1->property_->windowMode_ = WindowMode::WINDOW_MODE_FLOATING;

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_NEGATIVE_SCREEN);
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession1->GetPersistentId(), sceneSession1));
    result = ssm_->GetNextFocusableSessionWhenFloatWindowExist(DEFAULT_DISPLAY_ID, sceneSession->GetPersistentId());
    EXPECT_EQ(result, nullptr);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sceneSession->property_->windowMode_ = WindowMode::WINDOW_MODE_FULLSCREEN;
    ssm_->systemConfig_.freeMultiWindowEnable_ = false;
    result = ssm_->GetNextFocusableSessionWhenFloatWindowExist(DEFAULT_DISPLAY_ID, sceneSession->GetPersistentId());
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->GetPersistentId(), sceneSession1->GetPersistentId());
    screenSessionManagerClient_->screenSessionMap_.clear();
}

} // namespace
} // namespace Rosen
} // namespace OHOS
