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
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "session_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const std::string EMPTY_DEVICE_ID = "";
using ConfigItem = WindowSceneConfig::ConfigItem;
}
class SceneSessionManagerTest7 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    static bool gestureNavigationEnabled_;
    static ProcessGestureNavigationEnabledChangeFunc callbackFunc_;
    static sptr<SceneSessionManager> ssm_;
private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerTest7::ssm_ = nullptr;

bool SceneSessionManagerTest7::gestureNavigationEnabled_ = true;
ProcessGestureNavigationEnabledChangeFunc SceneSessionManagerTest7::callbackFunc_ = [](bool enable) {
    gestureNavigationEnabled_ = enable;
};

void WindowChangedFuncTest(int32_t persistentId, WindowUpdateType type)
{
}

void ProcessStatusBarEnabledChangeFuncTest(bool enable)
{
}

void DumpRootSceneElementInfoFuncTest(const std::vector<std::string>& params, std::vector<std::string>& infos)
{
}

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
    usleep(WAIT_SYNC_IN_NS);
    ssm_->sceneSessionMap_.clear();
}

namespace {
/**
 * @tc.name: UpdateSessionWindowVisibilityListener
 * @tc.desc: UpdateSessionWindowVisibilityListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, UpdateSessionWindowVisibilityListener, Function | SmallTest | Level3)
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
HWTEST_F(SceneSessionManagerTest7, ProcessVirtualPixelRatioChange, Function | SmallTest | Level3)
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
HWTEST_F(SceneSessionManagerTest7, ProcessVirtualPixelRatioChange01, Function | SmallTest | Level3)
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
HWTEST_F(SceneSessionManagerTest7, ProcessUpdateRotationChange, Function | SmallTest | Level3)
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
HWTEST_F(SceneSessionManagerTest7, FlushUIParams, Function | SmallTest | Level3)
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
HWTEST_F(SceneSessionManagerTest7, FlushUIParams01, Function | SmallTest | Level3)
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
 * @tc.name: RegisterIAbilityManagerCollaborator
 * @tc.desc: RegisterIAbilityManagerCollaborator
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, RegisterIAbilityManagerCollaborator, Function | SmallTest | Level3)
{
    int32_t type = 0;
    sptr<AAFwk::IAbilityManagerCollaborator> impl = nullptr;
    ASSERT_NE(nullptr, ssm_);
    auto ret = ssm_->RegisterIAbilityManagerCollaborator(type, impl);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: ProcessBackEvent
 * @tc.desc: ProcessBackEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, ProcessBackEvent, Function | SmallTest | Level3)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "FlushUIParams01";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ssm_->focusedSessionId_ = 1;
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = true;
    auto ret = ssm_->ProcessBackEvent();
    EXPECT_EQ(ret, WSError::WS_OK);
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = false;
    sessionInfo.isSystem_ = true;
    ssm_->rootSceneProcessBackEventFunc_ = nullptr;
    ret = ssm_->ProcessBackEvent();
    EXPECT_EQ(ret, WSError::WS_OK);
    RootSceneProcessBackEventFunc func = [](){};
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
HWTEST_F(SceneSessionManagerTest7, DestroySpecificSession, Function | SmallTest | Level3)
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
HWTEST_F(SceneSessionManagerTest7, DestroyAndDisconnectSpecificSessionWithDetachCallback,
    Function | SmallTest | Level3)
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
HWTEST_F(SceneSessionManagerTest7, DestroyAndDisconnectSpecificSessionInner, Function | SmallTest | Level3)
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
HWTEST_F(SceneSessionManagerTest7, DestroyAndDisconnectSpecificSessionInner01, Function | SmallTest | Level3)
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
HWTEST_F(SceneSessionManagerTest7, DestroyAndDisconnectSpecificSessionInner02, Function | SmallTest | Level3)
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
 * @tc.name: CheckPiPPriority
 * @tc.desc: CheckPiPPriority
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, CheckPiPPriority, Function | SmallTest | Level3)
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
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    auto ret = ssm_->CheckPiPPriority(pipTemplateInfo);
    EXPECT_EQ(ret, false);
    sceneSession->isVisible_ = false;
    ret = ssm_->CheckPiPPriority(pipTemplateInfo);
    EXPECT_EQ(ret, true);
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
HWTEST_F(SceneSessionManagerTest7, IsSessionVisibleForeground, Function | SmallTest | Level3)
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
 * @tc.name: GetAllSessionDumpInfo
 * @tc.desc: GetAllSessionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, GetAllSessionDumpInfo, Function | SmallTest | Level3)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "GetAllSessionDumpInfo";
    sessionInfo.isSystem_ = false;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    std::string dumpInfo = "";
    auto ret = ssm_->GetAllSessionDumpInfo(dumpInfo);
    EXPECT_EQ(ret, WSError::WS_OK);
    sceneSession->state_ = SessionState::STATE_END;
    ret = ssm_->GetAllSessionDumpInfo(dumpInfo);
    EXPECT_EQ(ret, WSError::WS_OK);
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    ret = ssm_->GetAllSessionDumpInfo(dumpInfo);
    EXPECT_EQ(ret, WSError::WS_OK);
    sessionInfo.isSystem_ = true;
    ret = ssm_->GetAllSessionDumpInfo(dumpInfo);
    EXPECT_EQ(ret, WSError::WS_OK);
    sceneSession = nullptr;
    ret = ssm_->GetAllSessionDumpInfo(dumpInfo);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: GetAllSessionDumpInfo01
 * @tc.desc: GetAllSessionDumpInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest7, GetAllSessionDumpInfo01, Function | SmallTest | Level3)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest7";
    sessionInfo.abilityName_ = "GetAllSessionDumpInfo01";
    sessionInfo.isSystem_ = true;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->isVisible_ = true;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    std::string dumpInfo = "";
    auto ret = ssm_->GetAllSessionDumpInfo(dumpInfo);
    EXPECT_EQ(ret, WSError::WS_OK);
    sceneSession->isVisible_ = false;
    ret = ssm_->GetAllSessionDumpInfo(dumpInfo);
    EXPECT_EQ(ret, WSError::WS_OK);
}
}
} // namespace Rosen
} // namespace OHOS