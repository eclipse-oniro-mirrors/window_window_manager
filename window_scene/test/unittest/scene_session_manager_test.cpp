/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "interfaces/include/ws_common.h"
#include "iremote_object_mocker.h"
#include "screen_fold_data.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/main_session.h"
#include "window_manager_agent.h"
#include "window_manager_hilog.h"
#include "session_manager.h"
#include "wm_common.h"
#include "zidl/window_manager_agent_interface.h"
#include "mock/mock_accesstoken_kit.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "application_info.h"
#include "context.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_logMsg += msg;
    }
}
namespace {
const std::string EMPTY_DEVICE_ID = "";
}
class SceneSessionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static void SetVisibleForAccessibility(sptr<SceneSession>& sceneSession);
    int32_t GetTaskCount(sptr<SceneSession>& session);
    static bool gestureNavigationEnabled_;
    static ProcessGestureNavigationEnabledChangeFunc callbackFunc_;
    static sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
    static constexpr uint32_t WAIT_SYNC_FOR_SNAPSHOT_SKIP_IN_NS = 500000;
    static constexpr uint32_t WAIT_SYNC_FOR_TEST_END_IN_NS = 1000000;
};

sptr<SceneSessionManager> SceneSessionManagerTest::ssm_ = nullptr;

bool SceneSessionManagerTest::gestureNavigationEnabled_ = true;
ProcessGestureNavigationEnabledChangeFunc SceneSessionManagerTest::callbackFunc_ =
    [](bool enable, const std::string& bundleName, GestureBackType type) { gestureNavigationEnabled_ = enable; };

void SceneSessionManagerTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerTest::SetUp()
{
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerTest::TearDown()
{
    MockAccesstokenKit::ChangeMockStateToInit();
    usleep(WAIT_SYNC_IN_NS);
    ssm_->sceneSessionMap_.clear();
    ssm_->screenPipEnabledMap_.clear();
}

void SceneSessionManagerTest::SetVisibleForAccessibility(sptr<SceneSession>& sceneSession)
{
    sceneSession->SetTouchable(true);
    sceneSession->forceTouchable_ = true;
    sceneSession->systemTouchable_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->foregroundInteractiveStatus_.store(true);
    sceneSession->isVisible_ = true;
}

int32_t SceneSessionManagerTest::GetTaskCount(sptr<SceneSession>& session)
{
    std::string dumpInfo = session->handler_->GetEventRunner()->GetEventQueue()->DumpCurrentQueueSize();
    std::regex pattern("\\d+");
    std::smatch matches;
    int32_t taskNum = 0;
    while (std::regex_search(dumpInfo, matches, pattern)) {
        taskNum += std::stoi(matches.str());
        dumpInfo = matches.suffix();
    }
    return taskNum;
}

namespace {
/**
 * @tc.name: SetBrightness
 * @tc.desc: ScreenSesionManager set session brightness
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, SetBrightness, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "SetBrightness";
    info.bundleName_ = "SetBrightness1";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    float brightness = 0.5;
    WSError result = ssm_->SetBrightness(sceneSession, brightness);
    EXPECT_EQ(result, WSError::WS_OK);
    EXPECT_NE(brightness, ssm_->GetDisplayBrightness());
}

/**
 * @tc.name: GerPrivacyBundleListTwoWindow
 * @tc.desc: get privacy bundle list when two windows exist.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GerPrivacyBundleListTwoWindow, TestSize.Level1)
{
    SessionInfo sessionInfoFirst;
    sessionInfoFirst.bundleName_ = "privacy.test.first";
    sessionInfoFirst.abilityName_ = "privacyAbilityName";
    sptr<SceneSession> sceneSessionFirst = ssm_->CreateSceneSession(sessionInfoFirst, nullptr);
    ASSERT_NE(sceneSessionFirst, nullptr);
    ssm_->sceneSessionMap_.insert({ sceneSessionFirst->GetPersistentId(), sceneSessionFirst });

    SessionInfo sessionInfoSecond;
    sessionInfoSecond.bundleName_ = "privacy.test.second";
    sessionInfoSecond.abilityName_ = "privacyAbilityName";
    sptr<SceneSession> sceneSessionSecond = ssm_->CreateSceneSession(sessionInfoSecond, nullptr);
    ASSERT_NE(sceneSessionSecond, nullptr);
    ssm_->sceneSessionMap_.insert({ sceneSessionSecond->GetPersistentId(), sceneSessionSecond });

    sceneSessionFirst->GetSessionProperty()->displayId_ = 0;
    sceneSessionFirst->GetSessionProperty()->isPrivacyMode_ = true;
    sceneSessionFirst->state_ = SessionState::STATE_FOREGROUND;

    sceneSessionSecond->GetSessionProperty()->displayId_ = 0;
    sceneSessionSecond->GetSessionProperty()->isPrivacyMode_ = true;
    sceneSessionSecond->state_ = SessionState::STATE_FOREGROUND;

    std::unordered_map<DisplayId, std::unordered_set<std::string>> privacyBundleList;
    ssm_->GetSceneSessionPrivacyModeBundles(0, privacyBundleList);
    EXPECT_EQ(privacyBundleList.size(), 1);
    EXPECT_EQ(privacyBundleList[0].size(), 2);

    sceneSessionSecond->GetSessionProperty()->displayId_ = 1;
    privacyBundleList.clear();
    ssm_->GetSceneSessionPrivacyModeBundles(0, privacyBundleList);
    EXPECT_EQ(privacyBundleList.size(), 1);

    privacyBundleList.clear();
    ssm_->GetSceneSessionPrivacyModeBundles(1, privacyBundleList);
    EXPECT_EQ(privacyBundleList.size(), 1);
}

/**
 * @tc.name: SetWindowFlags01
 * @tc.desc: SceneSesionManager set window flags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, SetWindowFlags01, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "bundleName";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    uint32_t flags = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    property->SetWindowFlags(flags);
    sptr<SceneSession> sceneSession = nullptr;
    WSError result01 = ssm_->SetWindowFlags(sceneSession, property);
    EXPECT_EQ(result01, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: SetWindowFlags02
 * @tc.desc: SceneSesionManager set window flags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, SetWindowFlags02, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "bundleName";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    uint32_t flags = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    property->SetWindowFlags(flags);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    WSError result02 = ssm_->SetWindowFlags(sceneSession, property);
    EXPECT_EQ(result02, WSError::WS_ERROR_NOT_SYSTEM_APP);
}

/**
 * @tc.name: SetWindowFlags03
 * @tc.desc: SceneSesionManager set window flags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, SetWindowFlags03, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "bundleName";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    uint32_t flags = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_SHOW_WHEN_LOCKED);
    property->SetWindowFlags(flags);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    property->SetSystemCalling(true);
    WSError result03 = ssm_->SetWindowFlags(sceneSession, property);
    ASSERT_EQ(result03, WSError::WS_OK);
}

/**
 * @tc.name: NotifyWaterMarkFlagChangedResult
 * @tc.desc: SceneSesionManager notify water mark flag changed result
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, NotifyWaterMarkFlagChangedResult, TestSize.Level1)
{
    int32_t persistentId = 10086;
    ssm_->NotifyCompleteFirstFrameDrawing(persistentId);
    bool hasWaterMark = true;
    AppExecFwk::AbilityInfo abilityInfo;
    WSError result01 = ssm_->NotifyWaterMarkFlagChangedResult(hasWaterMark);
    EXPECT_EQ(result01, WSError::WS_OK);
    ssm_->CheckAndNotifyWaterMarkChangedResult();
    ssm_->ProcessPreload(abilityInfo);
}

/**
 * @tc.name: IsValidSessionIds
 * @tc.desc: SceneSesionManager is valid session id
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, IsValidSessionIds, TestSize.Level1)
{
    std::vector<int32_t> sessionIds = { 0, 1, 2, 3, 4, 5, 24, 10086 };
    std::vector<bool> results = {};
    WSError result = ssm_->IsValidSessionIds(sessionIds, results);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: UnRegisterSessionListener
 * @tc.desc: SceneSesionManager un register session listener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, UnRegisterSessionListener, TestSize.Level1)
{
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    OHOS::MessageParcel data;
    sptr<ISessionListener> listener = iface_cast<ISessionListener>(data.ReadRemoteObject());
    WSError result = ssm_->UnRegisterSessionListener(listener);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: GetSessionInfos01
 * @tc.desc: SceneSesionManager get session infos
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetSessionInfos01, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSACalling(false);
    std::string deviceId = "1245";
    int32_t numMax = 1024;
    AAFwk::MissionInfo infoFrist;
    infoFrist.label = "fristBundleName";
    AAFwk::MissionInfo infoSecond;
    infoSecond.label = "secondBundleName";
    std::vector<SessionInfoBean> sessionInfos = { infoFrist, infoSecond };
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    WSError result = ssm_->GetSessionInfos(deviceId, numMax, sessionInfos);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: GetSessionInfos02
 * @tc.desc: SceneSesionManager get session infos
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetSessionInfos02, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSACalling(false);
    std::string deviceId = "1245";
    AAFwk::MissionInfo infoFrist;
    infoFrist.label = "fristBundleName";
    AAFwk::MissionInfo infoSecond;
    infoSecond.label = "secondBundleName";
    std::vector<SessionInfoBean> sessionInfos = { infoFrist, infoSecond };
    MockAccesstokenKit::MockAccessTokenKitRet(0);
    int32_t persistentId = 24;
    SessionInfoBean sessionInfo;
    int result01 = ssm_->GetRemoteSessionInfo(deviceId, persistentId, sessionInfo);
    ASSERT_NE(result01, ERR_OK);
}

/**
 * @tc.name: GetUnreliableWindowInfo
 * @tc.desc: SceneSesionManager get unreliable window info
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetUnreliableWindowInfo, TestSize.Level1)
{
    int32_t windowId = 0;
    std::vector<sptr<UnreliableWindowInfo>> infos;
    WMError result = ssm_->GetUnreliableWindowInfo(windowId, infos);
    EXPECT_EQ(WMError::WM_OK, result);
}

/**
 * @tc.name: GetMainWindowStatesByPid
 * @tc.desc: SceneSesionManager get main window states
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetMainWindowStatesByPid, TestSize.Level1)
{
    int32_t pid = 100;
    std::vector<MainWindowState> windowStates;
    WSError result = ssm_->GetMainWindowStatesByPid(pid, windowStates);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: CheckIsRemote01
 * @tc.desc: DeviceId is empty
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, CheckIsRemote01, TestSize.Level1)
{
    std::string deviceId;
    EXPECT_EQ(deviceId.empty(), true);
    bool result = ssm_->CheckIsRemote(deviceId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: AnonymizeDeviceId01
 * @tc.desc: SceneSesionManager anonymize deviceId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, AnonymizeDeviceId01, TestSize.Level1)
{
    std::string deviceId;
    std::string result(ssm_->AnonymizeDeviceId(deviceId));
    EXPECT_EQ(result, EMPTY_DEVICE_ID);
}

/**
 * @tc.name: AnonymizeDeviceId02
 * @tc.desc: SceneSesionManager anonymize deviceId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, AnonymizeDeviceId02, TestSize.Level1)
{
    std::string deviceId;
    deviceId.assign("100964857");
    std::string result01 = "100964******";
    ASSERT_EQ(ssm_->AnonymizeDeviceId(deviceId), result01);
}

/**
 * @tc.name: TerminateSessionNew01
 * @tc.desc: SceneSesionManager terminate session new
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, TerminateSessionNew01, TestSize.Level1)
{
    sptr<AAFwk::SessionInfo> info = nullptr;
    bool needStartCaller = true;
    WSError result01 = ssm_->TerminateSessionNew(info, needStartCaller);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, result01);
}

/**
 * @tc.name: TerminateSessionNew02
 * @tc.desc: SceneSesionManager terminate session new
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, TerminateSessionNew, TestSize.Level1)
{
    bool needStartCaller = true;
    sptr<AAFwk::SessionInfo> info = sptr<AAFwk::SessionInfo>::MakeSptr();
    WSError result02 = ssm_->TerminateSessionNew(info, needStartCaller);
    EXPECT_EQ(WSError::WS_ERROR_INVALID_PARAM, result02);
}

/**
 * @tc.name: RegisterSessionListener01
 * @tc.desc: SceneSesionManager register session listener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, RegisterSessionListener01, TestSize.Level1)
{
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    OHOS::MessageParcel data;
    sptr<ISessionListener> listener = iface_cast<ISessionListener>(data.ReadRemoteObject());
    WSError result = ssm_->RegisterSessionListener(listener);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: ClearDisplayStatusBarTemporarilyFlags01
 * @tc.desc: check ClearDisplayStatusBarTemporarilyFlags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, ClearDisplayStatusBarTemporarilyFlags01, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "ClearDisplayStatusBarTemporarilyFlags";
    sessionInfo.abilityName_ = "ClearDisplayStatusBarTemporarilyFlags";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<SceneSession> sceneSession = ssm_->RequestSceneSession(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
}

/**
 * @tc.name: ClearDisplayStatusBarTemporarilyFlags02
 * @tc.desc: check ClearDisplayStatusBarTemporarilyFlags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, ClearDisplayStatusBarTemporarilyFlags02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "ClearDisplayStatusBarTemporarilyFlags";
    sessionInfo.abilityName_ = "ClearDisplayStatusBarTemporarilyFlags";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<SceneSession> sceneSession = ssm_->RequestSceneSession(sessionInfo, nullptr);
    sceneSession->SetIsDisplayStatusBarTemporarily(true);
    ASSERT_EQ(true, sceneSession->GetIsDisplayStatusBarTemporarily());
}

/**
 * @tc.name: ClearDisplayStatusBarTemporarilyFlags03
 * @tc.desc: check ClearDisplayStatusBarTemporarilyFlags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, ClearDisplayStatusBarTemporarilyFlags03, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "ClearDisplayStatusBarTemporarilyFlags";
    sessionInfo.abilityName_ = "ClearDisplayStatusBarTemporarilyFlags";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<SceneSession> sceneSession = ssm_->RequestSceneSession(sessionInfo, nullptr);
    sceneSession->SetIsDisplayStatusBarTemporarily(true);
    ssm_->ClearDisplayStatusBarTemporarilyFlags();
    ASSERT_EQ(true, sceneSession->GetIsDisplayStatusBarTemporarily());
}

/**
 * @tc.name: RequestSceneSessionByCall01
 * @tc.desc: SceneSesionManager request scene session by call
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, RequestSceneSessionByCall01, TestSize.Level1)
{
    sptr<SceneSession> sceneSession = nullptr;
    WSError result01 = ssm_->RequestSceneSessionByCall(nullptr);
    EXPECT_EQ(result01, WSError::WS_OK);
}

/**
 * @tc.name: RequestSceneSessionByCall02
 * @tc.desc: SceneSesionManager request scene session by call
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, RequestSceneSessionByCall02, TestSize.Level1)
{
    sptr<SceneSession> sceneSession = nullptr;
    SessionInfo info;
    info.bundleName_ = "bundleName";
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    WSError result02 = ssm_->RequestSceneSessionByCall(sceneSession);
    EXPECT_EQ(result02, WSError::WS_OK);
}

/**
 * @tc.name: StartAbilityBySpecified
 * @tc.desc: SceneSesionManager start ability by specified
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, StartAbilityBySpecified, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    SessionInfo info;
    ssm_->StartAbilityBySpecified(info);

    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    AAFwk::WantParams wantParams;
    want->SetParams(wantParams);
    info.want = want;
    ssm_->StartAbilityBySpecified(info);
    EXPECT_FALSE(g_logMsg.find("start specified ability by SCB result") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: FindMainWindowWithToken01
 * @tc.desc: SceneSesionManager find main window with token
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, FindMainWindowWithToken01, TestSize.Level1)
{
    sptr<IRemoteObject> targetToken = nullptr;
    sptr<SceneSession> result = ssm_->FindMainWindowWithToken(targetToken);
    ASSERT_EQ(result, nullptr);

    uint64_t persistentId = 1423;
    WSError result01 = ssm_->BindDialogSessionTarget(persistentId, targetToken);
    EXPECT_EQ(result01, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: FindMainWindowWithToken02
 * @tc.desc: SceneSesionManager find main window with token
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, FindMainWindowWithToken02, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.moduleName_ = "test3";
    info.persistentId_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
}

/**
 * @tc.name: FindMainWindowWithToken03
 * @tc.desc: SceneSesionManager find main window with token
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, FindMainWindowWithToken03, TestSize.Level1)
{
    sptr<IRemoteObject> targetToken = sptr<IRemoteObjectMocker>::MakeSptr();
    uint64_t persistentId = 1423;
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.moduleName_ = "test3";
    info.persistentId_ = 1;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    persistentId = 1;
    WSError result02 = ssm_->BindDialogSessionTarget(persistentId, targetToken);
    EXPECT_EQ(result02, WSError::WS_OK);

    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    WSError result03 = ssm_->BindDialogSessionTarget(persistentId, targetToken);
    EXPECT_EQ(result03, WSError::WS_OK);
}

/**
 * @tc.name: UpdateParentSessionForDialog001
 * @tc.desc: SceneSesionManager update parent session for dialog
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, UpdateParentSessionForDialog001, TestSize.Level1)
{
    SessionInfo dialogInfo;
    dialogInfo.abilityName_ = "DialogWindows";
    dialogInfo.bundleName_ = "DialogWindows";
    SessionInfo parentInfo;
    parentInfo.abilityName_ = "ParentWindows";
    parentInfo.bundleName_ = "ParentWindows";

    int32_t persistentId = 1005;
    sptr<SceneSession> parentSession = sptr<SceneSession>::MakeSptr(parentInfo, nullptr);
    ASSERT_NE(parentSession, nullptr);
    ssm_->sceneSessionMap_.insert({ persistentId, parentSession });

    sptr<SceneSession> dialogSession = sptr<SceneSession>::MakeSptr(dialogInfo, nullptr);
    ASSERT_NE(dialogSession, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetParentPersistentId(persistentId);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);

    WSError result = ssm_->UpdateParentSessionForDialog(dialogSession, property);
    EXPECT_EQ(dialogSession->GetParentPersistentId(), persistentId);
    EXPECT_NE(dialogSession->GetParentSession(), nullptr);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: IsFreeMultiWindow
 * @tc.desc: IsFreeMultiWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, IsFreeMultiWindow, TestSize.Level1)
{
    bool isFreeMultiWindow = false;
    // freeMultiWindowEnable false
    ssm_->systemConfig_.freeMultiWindowEnable_ = false;
    auto result = ssm_->IsFreeMultiWindow(isFreeMultiWindow);
    EXPECT_EQ(result, WMError::WM_OK);
    
    // freeMultiWindowEnable true
    ssm_->systemConfig_.freeMultiWindowEnable_ = true;
    result = ssm_->IsFreeMultiWindow(isFreeMultiWindow);
    EXPECT_EQ(result, WMError::WM_OK);
}

/**
 * @tc.name: MoveSessionsToBackground01
 * @tc.desc: SceneSesionManager move sessions to background
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, MoveSessionsToBackground01, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSACalling(false);
    int32_t type = CollaboratorType::RESERVE_TYPE;
    WSError result01 = ssm_->UnregisterIAbilityManagerCollaborator(type);
    EXPECT_EQ(result01, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: MoveSessionsToBackground02
 * @tc.desc: SceneSesionManager move sessions to background
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, MoveSessionsToBackground02, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSACalling(false);
    std::vector<std::int32_t> sessionIds = { 1, 2, 3, 15, 1423 };
    std::vector<int32_t> res = { 1, 2, 3, 15, 1423 };
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    WSError result03 = ssm_->MoveSessionsToBackground(sessionIds, res);
    EXPECT_EQ(result03, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: ClearCollaboratorSessionsByType01
 * @tc.desc: SceneSesionManager clear all collaborator sessions
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, ClearCollaboratorSessionsByType01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::string bundleName = "bundleName";
    std::string abilityName = "abilityName";
    int32_t persistentId = 1200;
    SessionInfo info;
    info.bundleName_ = bundleName;
    info.abilityName_ = abilityName;
    info.persistentId_ = persistentId;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->SetCollaboratorType(CollaboratorType::REDIRECT_TYPE);
    sceneSession->SetTerminateSessionListenerNew(
        [](const SessionInfo &info, bool needStartCaller, bool isFromBroker, bool isForceClean) {
            ssm_->sceneSessionMap_.erase(info.persistentId_);
        });
    usleep(WAIT_SYNC_IN_NS);
    ssm_->sceneSessionMap_.insert({persistentId, sceneSession});
    ssm_->ClearCollaboratorSessionsByType(CollaboratorType::DEFAULT_TYPE);
    EXPECT_FALSE(g_logMsg.find("clear session id") != std::string::npos);
    ssm_->ClearCollaboratorSessionsByType(CollaboratorType::REDIRECT_TYPE);
    EXPECT_TRUE(g_logMsg.find("clear session id") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: MoveSessionsToForeground
 * @tc.desc: SceneSesionManager move sessions to foreground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, MoveSessionsToForeground, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSACalling(false);
    std::vector<std::int32_t> sessionIds = { 1, 2, 3, 15, 1423 };
    int32_t topSessionId = 1;
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    WSError result = ssm_->MoveSessionsToForeground(sessionIds, topSessionId);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: UnlockSession
 * @tc.desc: SceneSesionManager unlock session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, UnlockSession, TestSize.Level1)
{
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    int32_t sessionId = 1;
    WSError result = ssm_->UnlockSession(sessionId);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PERMISSION);
    result = ssm_->LockSession(sessionId);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: GetImmersiveState
 * @tc.desc: test GetImmersiveState
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetImmersiveState, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ScreenId screenId = 1;
    auto ret = ssm_->GetImmersiveState(screenId);
    EXPECT_NE(ret, true);
    EXPECT_FALSE(g_logMsg.find("session is nullptr") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: NotifyAINavigationBarShowStatus
 * @tc.desc: test NotifyAINavigationBarShowStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, NotifyAINavigationBarShowStatus, TestSize.Level1)
{
    bool isVisible = false;
    WSRect barArea = { 0, 0, 320, 240 }; // width: 320, height: 240
    uint64_t displayId = 0;
    ssm_->rootSceneSession_ = sptr<RootSceneSession>::MakeSptr();
    WSError result = ssm_->NotifyAINavigationBarShowStatus(isVisible, barArea, displayId);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: NotifyWindowExtensionVisibilityChange
 * @tc.desc: test NotifyWindowExtensionVisibilityChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, NotifyWindowExtensionVisibilityChange, TestSize.Level1)
{
    int32_t pid = getprocpid();
    int32_t uid = getuid();
    bool isVisible = false;
    WSError result = ssm_->NotifyWindowExtensionVisibilityChange(pid, uid, isVisible);
    EXPECT_EQ(result, WSError::WS_OK);

    pid = INVALID_PID;
    uid = INVALID_USER_ID;
    result = ssm_->NotifyWindowExtensionVisibilityChange(pid, uid, isVisible);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: UpdateTopmostProperty
 * @tc.desc: test UpdateTopmostProperty
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, UpdateTopmostProperty, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateTopmostProperty";
    info.bundleName_ = "UpdateTopmostProperty";
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetTopmost(true);
    property->SetSystemCalling(true);
    WMError result = ssm_->UpdateTopmostProperty(property, sceneSession);
    EXPECT_EQ(WMError::WM_OK, result);
}

/**
 * @tc.name: UpdateSessionWindowVisibilityListener
 * @tc.desc: SceneSesionManager update window visibility listener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, UpdateSessionWindowVisibilityListener, TestSize.Level1)
{
    int32_t persistentId = 10086;
    bool haveListener = true;
    WSError result = ssm_->UpdateSessionWindowVisibilityListener(persistentId, haveListener);
    EXPECT_EQ(result, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: GetSessionSnapshotById
 * @tc.desc: test GetSessionSnapshotById
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetSessionSnapshotById, TestSize.Level1)
{
    int32_t persistentId = -1;
    SessionSnapshot snapshot;
    WMError ret = ssm_->GetSessionSnapshotById(persistentId, snapshot);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: GetUIContentRemoteObj
 * @tc.desc: SceneSesionManager GetUIContentRemoteObj
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetUIContentRemoteObj, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObj;
    EXPECT_EQ(ssm_->GetUIContentRemoteObj(65535, remoteObj), WSError::WS_ERROR_INVALID_PERMISSION);
    SessionInfo info;
    info.abilityName_ = "GetUIContentRemoteObj";
    info.bundleName_ = "GetUIContentRemoteObj";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ 65535, sceneSession });
    EXPECT_EQ(ssm_->GetUIContentRemoteObj(65535, remoteObj), WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: GetRootUIContentRemoteObj
 * @tc.desc: get remote object of root UIContent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetRootUIContentRemoteObj, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IRemoteObject> remoteObj;
    EXPECT_EQ(ssm_->GetRootUIContentRemoteObj(0, remoteObj), WMError::WM_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: GetRootUIContentRemoteObjInner
 * @tc.desc: get remote object of root UIContent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetRootUIContentRemoteObjInner, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    auto oldRootSession = ssm_->rootSceneSession_;
    DisplayId displayId = 0;
    sptr<IRemoteObject> remoteObj;
    ssm_->rootSceneSession_ = nullptr;
    EXPECT_EQ(ssm_->GetRootUIContentRemoteObjInner(displayId, remoteObj), WMError::WM_ERROR_INVALID_WINDOW);
    auto specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    ssm_->rootSceneSession_ = sptr<RootSceneSession>::MakeSptr(specificCb);
    auto oldFoldStatus = PcFoldScreenManager::GetInstance().screenFoldStatus_;
    auto oldDisplayId = PcFoldScreenManager::GetInstance().displayId_;
    PcFoldScreenManager::GetInstance().screenFoldStatus_ = SuperFoldStatus::HALF_FOLDED;
    PcFoldScreenManager::GetInstance().displayId_ = displayId;
    EXPECT_EQ(ssm_->GetRootUIContentRemoteObjInner(displayId, remoteObj), WMError::WM_ERROR_NULLPTR);
    PcFoldScreenManager::GetInstance().screenFoldStatus_ = oldFoldStatus;
    PcFoldScreenManager::GetInstance().displayId_ = oldDisplayId;
    ssm_->rootSceneSession_ = oldRootSession;
}

/**
 * @tc.name: CalculateCombinedExtWindowFlags
 * @tc.desc: SceneSesionManager calculate combined extension window flags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, CalculateCombinedExtWindowFlags, TestSize.Level1)
{
    EXPECT_EQ(ssm_->combinedExtWindowFlags_.bitData, 0);
    ssm_->UpdateSpecialExtWindowFlags(1234, ExtensionWindowFlags(3), ExtensionWindowFlags(3));
    ssm_->UpdateSpecialExtWindowFlags(5678, ExtensionWindowFlags(4), ExtensionWindowFlags(4));
    ssm_->CalculateCombinedExtWindowFlags();
    EXPECT_EQ(ssm_->combinedExtWindowFlags_.bitData, 7);
    ssm_->extWindowFlagsMap_.clear();
}

/**
 * @tc.name: UpdateSpecialExtWindowFlags
 * @tc.desc: SceneSesionManager update special extension window flags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, UpdateSpecialExtWindowFlags, TestSize.Level1)
{
    int32_t persistentId = 12345;
    EXPECT_TRUE(ssm_->extWindowFlagsMap_.empty());
    ssm_->UpdateSpecialExtWindowFlags(persistentId, 3, 3);
    EXPECT_EQ(ssm_->extWindowFlagsMap_.size(), 1);
    EXPECT_EQ(ssm_->extWindowFlagsMap_.begin()->first, persistentId);
    EXPECT_EQ(ssm_->extWindowFlagsMap_.begin()->second.bitData, 3);
    ssm_->UpdateSpecialExtWindowFlags(persistentId, 0, 3);
    EXPECT_TRUE(ssm_->extWindowFlagsMap_.empty());
    ssm_->extWindowFlagsMap_.clear();
}

/**
 * @tc.name: HideNonSecureFloatingWindows
 * @tc.desc: SceneSesionManager hide non-secure floating windows
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, HideNonSecureFloatingWindows, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HideNonSecureFloatingWindows";
    info.bundleName_ = "HideNonSecureFloatingWindows";

    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession->GetPersistentId(), sceneSession));

    sptr<SceneSession> floatSession;
    floatSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(floatSession, nullptr);
    floatSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    ssm_->nonSystemFloatSceneSessionMap_.insert(std::make_pair(floatSession->GetPersistentId(), floatSession));

    EXPECT_FALSE(ssm_->shouldHideNonSecureFloatingWindows_.load());
    EXPECT_FALSE(floatSession->GetSessionProperty()->GetForceHide());
    sceneSession->combinedExtWindowFlags_.hideNonSecureWindowsFlag = true;
    ssm_->HideNonSecureFloatingWindows();
    EXPECT_TRUE(floatSession->GetSessionProperty()->GetForceHide());
    sceneSession->combinedExtWindowFlags_.hideNonSecureWindowsFlag = false;
    ssm_->combinedExtWindowFlags_.hideNonSecureWindowsFlag = true;
    ssm_->HideNonSecureFloatingWindows();
    EXPECT_TRUE(floatSession->GetSessionProperty()->GetForceHide());

    ssm_->combinedExtWindowFlags_.hideNonSecureWindowsFlag = false;
    ssm_->HideNonSecureFloatingWindows();
    ssm_->combinedExtWindowFlags_.hideNonSecureWindowsFlag = true;
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ssm_->HideNonSecureFloatingWindows();
    EXPECT_FALSE(floatSession->GetSessionProperty()->GetForceHide());
    ssm_->systemConfig_.windowUIType_ = WindowUIType::INVALID_WINDOW;

    ssm_->shouldHideNonSecureFloatingWindows_.store(false);
    ssm_->sceneSessionMap_.clear();
    ssm_->nonSystemFloatSceneSessionMap_.clear();
}

/**
 * @tc.name: HideNonSecureSubWindows
 * @tc.desc: SceneSesionManager hide non-secure sub windows
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, HideNonSecureSubWindows, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HideNonSecureSubWindows";
    info.bundleName_ = "HideNonSecureSubWindows";

    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->combinedExtWindowFlags_.hideNonSecureWindowsFlag = true;

    sptr<SceneSession> subSession;
    subSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(subSession, nullptr);
    ASSERT_NE(subSession->GetSessionProperty(), nullptr);
    sceneSession->AddSubSession(subSession);
    subSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    subSession->GetSessionProperty()->SetParentPersistentId(sceneSession->GetPersistentId());
    ssm_->sceneSessionMap_.insert(std::make_pair(subSession->GetPersistentId(), subSession));

    EXPECT_FALSE(subSession->GetSessionProperty()->GetForceHide());
    sceneSession->combinedExtWindowFlags_.hideNonSecureWindowsFlag = true;
    ssm_->HideNonSecureSubWindows(sceneSession);
    EXPECT_TRUE(subSession->GetSessionProperty()->GetForceHide());
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: HandleSecureSessionShouldHide
 * @tc.desc: SceneSesionManager handle secure session should hide
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, HandleSecureSessionShouldHide, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HandleSecureSessionShouldHide";
    info.bundleName_ = "HandleSecureSessionShouldHide";

    sptr<SceneSession> sceneSession;
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->combinedExtWindowFlags_.hideNonSecureWindowsFlag = true;

    sptr<SceneSession> subSession;
    subSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(subSession, nullptr);
    ASSERT_NE(subSession->GetSessionProperty(), nullptr);
    sceneSession->AddSubSession(subSession);
    subSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    subSession->GetSessionProperty()->SetParentPersistentId(sceneSession->GetPersistentId());
    ssm_->sceneSessionMap_.insert(std::make_pair(subSession->GetPersistentId(), subSession));

    sptr<SceneSession> floatSession;
    floatSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(floatSession, nullptr);
    ASSERT_NE(floatSession->GetSessionProperty(), nullptr);
    floatSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    ssm_->nonSystemFloatSceneSessionMap_.insert(std::make_pair(floatSession->GetPersistentId(), floatSession));

    sceneSession->SetShouldHideNonSecureWindows(true);
    auto ret = ssm_->HandleSecureSessionShouldHide(sceneSession);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_TRUE(subSession->GetSessionProperty()->GetForceHide());
    EXPECT_TRUE(floatSession->GetSessionProperty()->GetForceHide());
    EXPECT_TRUE(ssm_->shouldHideNonSecureFloatingWindows_.load());
    ssm_->sceneSessionMap_.clear();
    ssm_->nonSystemFloatSceneSessionMap_.clear();
}

/**
 * @tc.name: HandleSpecialExtWindowFlagsChange
 * @tc.desc: SceneSesionManager handle special uiextension window flags change
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, HandleSpecialExtWindowFlagsChange, TestSize.Level1)
{
    int32_t persistentId = 12345;
    EXPECT_TRUE(ssm_->extWindowFlagsMap_.empty());
    ssm_->HandleSpecialExtWindowFlagsChange(persistentId, 3, 3);
    EXPECT_EQ(ssm_->extWindowFlagsMap_.size(), 1);
    EXPECT_EQ(ssm_->extWindowFlagsMap_.begin()->first, persistentId);
    EXPECT_EQ(ssm_->extWindowFlagsMap_.begin()->second.bitData, 3);
    ssm_->HandleSpecialExtWindowFlagsChange(persistentId, 0, 3);
    EXPECT_TRUE(ssm_->extWindowFlagsMap_.empty());
    ssm_->extWindowFlagsMap_.clear();
}

/**
 * @tc.name: UpdateModalExtensionRect
 * @tc.desc: SceneSesionManager update modal extension rect
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, UpdateModalExtensionRect, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "UpdateModalExtensionRect";
    info.bundleName_ = "UpdateModalExtensionRect";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    Rect rect{ 1, 2, 3, 4 };
    ssm_->UpdateModalExtensionRect(nullptr, rect);
    EXPECT_FALSE(sceneSession->GetLastModalUIExtensionEventInfo());
}

/**
 * @tc.name: ProcessModalExtensionPointDown
 * @tc.desc: SceneSesionManager process modal extension point down
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, ProcessModalExtensionPointDown, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "ProcessModalExtensionPointDown";
    info.bundleName_ = "ProcessModalExtensionPointDown";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    ssm_->ProcessModalExtensionPointDown(nullptr, 0, 0);
    EXPECT_FALSE(sceneSession->GetLastModalUIExtensionEventInfo());
}

/**
 * @tc.name: GetExtensionWindowIds
 * @tc.desc: SceneSesionManager get extension window ids
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetExtensionWindowIds, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetExtensionWindowIds";
    info.bundleName_ = "GetExtensionWindowIds";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    int32_t persistentId = 0;
    int32_t parentId = 0;
    EXPECT_FALSE(ssm_->GetExtensionWindowIds(nullptr, persistentId, parentId));
}

/**
 * @tc.name: AddOrRemoveSecureSession
 * @tc.desc: SceneSesionManager hide non-secure windows by scene session
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, AddOrRemoveSecureSession, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "AddOrRemoveSecureSession";
    info.bundleName_ = "AddOrRemoveSecureSession1";

    int32_t persistentId = 12345;
    auto ret = ssm_->AddOrRemoveSecureSession(persistentId, true);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: UpdateExtWindowFlags
 * @tc.desc: SceneSesionManager update uiextension window flags
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, UpdateExtWindowFlags, TestSize.Level1)
{
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    SessionInfo info;
    info.abilityName_ = "UpdateExtWindowFlags";
    info.bundleName_ = "UpdateExtWindowFlags";

    auto ret = ssm_->UpdateExtWindowFlags(nullptr, 7, 7);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: SetScreenLocked001
 * @tc.desc: SetScreenLocked001
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, SetScreenLocked001, TestSize.Level1)
{
    sptr<SceneSession> sceneSession = nullptr;
    SessionInfo info;
    info.bundleName_ = "bundleName";
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetEventHandler(ssm_->taskScheduler_->GetEventHandler(), ssm_->eventHandler_);
    ssm_->sceneSessionMap_.insert(std::make_pair(sceneSession->GetPersistentId(), sceneSession));
    DetectTaskInfo detectTaskInfo;
    detectTaskInfo.taskState = DetectTaskState::ATTACH_TASK;
    detectTaskInfo.taskWindowMode = WindowMode::WINDOW_MODE_UNDEFINED;
    sceneSession->SetDetectTaskInfo(detectTaskInfo);
    std::string taskName = "wms:WindowStateDetect" + std::to_string(sceneSession->persistentId_);
    auto task = []() {};
    int64_t delayTime = 3000;
    sceneSession->handler_->PostTask(task, taskName, delayTime);
    int32_t beforeTaskNum = GetTaskCount(sceneSession);
    ssm_->SetScreenLocked(true);
    sleep(1);
    EXPECT_EQ(beforeTaskNum - 1, GetTaskCount(sceneSession));
    EXPECT_EQ(DetectTaskState::NO_TASK, sceneSession->detectTaskInfo_.taskState);
    EXPECT_EQ(WindowMode::WINDOW_MODE_UNDEFINED, sceneSession->detectTaskInfo_.taskWindowMode);
}

/**
 * @tc.name: AccessibilityFillEmptySceneSessionListToNotifyList
 * @tc.desc: SceneSesionManager fill empty scene session list to accessibilityList;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, AccessibilityFillEmptySceneSessionListToNotifyList, TestSize.Level1)
{
    std::vector<sptr<SceneSession>> sceneSessionList;
    std::vector<sptr<AccessibilityWindowInfo>> accessibilityInfo;

    ssm_->FillAccessibilityInfo(sceneSessionList, accessibilityInfo);
    EXPECT_EQ(accessibilityInfo.size(), 0);
}

/**
 * @tc.name: AccessibilityFillOneSceneSessionListToNotifyList
 * @tc.desc: SceneSesionManager fill one sceneSession to accessibilityList;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, AccessibilityFillOneSceneSessionListToNotifyList, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "accessibilityNotifyTesterBundleName";
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";

    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    SetVisibleForAccessibility(sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    std::vector<sptr<SceneSession>> sceneSessionList;
    ssm_->GetAllSceneSessionForAccessibility(sceneSessionList);
    EXPECT_EQ(sceneSessionList.size(), 1);

    std::vector<sptr<AccessibilityWindowInfo>> accessibilityInfo;
    ssm_->FillAccessibilityInfo(sceneSessionList, accessibilityInfo);
    EXPECT_EQ(accessibilityInfo.size(), 1);
}

/**
 * @tc.name: AccessibilityFillTwoSceneSessionListToNotifyList
 * @tc.desc: SceneSesionManager fill two sceneSessions to accessibilityList;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, AccessibilityFillTwoSceneSessionListToNotifyList, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "accessibilityNotifyTesterBundleName";
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";

    sptr<SceneSession> sceneSessionFirst = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSessionFirst, nullptr);
    SetVisibleForAccessibility(sceneSessionFirst);

    sptr<SceneSession> sceneSessionSecond = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSessionSecond, nullptr);
    SetVisibleForAccessibility(sceneSessionSecond);

    ssm_->sceneSessionMap_.insert({ sceneSessionFirst->GetPersistentId(), sceneSessionFirst });
    ssm_->sceneSessionMap_.insert({ sceneSessionSecond->GetPersistentId(), sceneSessionSecond });

    std::vector<sptr<SceneSession>> sceneSessionList;
    ssm_->GetAllSceneSessionForAccessibility(sceneSessionList);
    EXPECT_EQ(sceneSessionList.size(), 2);

    std::vector<sptr<AccessibilityWindowInfo>> accessibilityInfo;
    ssm_->FillAccessibilityInfo(sceneSessionList, accessibilityInfo);
    EXPECT_EQ(accessibilityInfo.size(), 2);
}

/**
 * @tc.name: AccessibilityFillEmptyBundleName
 * @tc.desc: SceneSesionManager fill empty bundle name to accessibilityInfo;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, AccessibilityFillEmptyBundleName, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";

    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    SetVisibleForAccessibility(sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    std::vector<sptr<SceneSession>> sceneSessionList;
    ssm_->GetAllSceneSessionForAccessibility(sceneSessionList);
    EXPECT_EQ(sceneSessionList.size(), 1);

    std::vector<sptr<AccessibilityWindowInfo>> accessibilityInfo;
    ssm_->FillAccessibilityInfo(sceneSessionList, accessibilityInfo);
    EXPECT_EQ(accessibilityInfo.size(), 1);

    EXPECT_EQ(accessibilityInfo.at(0)->bundleName_, "");
    EXPECT_EQ(sceneSessionList.at(0)->GetSessionInfo().bundleName_, "");
    EXPECT_EQ(accessibilityInfo.at(0)->bundleName_, sceneSessionList.at(0)->GetSessionInfo().bundleName_);
}

/**
 * @tc.name: AccessibilityFillBundleName
 * @tc.desc: SceneSesionManager fill bundle name to accessibilityInfo;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, AccessibilityFillBundleName, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "accessibilityNotifyTesterBundleName";
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";

    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    SetVisibleForAccessibility(sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    std::vector<sptr<SceneSession>> sceneSessionList;
    ssm_->GetAllSceneSessionForAccessibility(sceneSessionList);
    EXPECT_EQ(sceneSessionList.size(), 1);

    std::vector<sptr<AccessibilityWindowInfo>> accessibilityInfo;
    ssm_->FillAccessibilityInfo(sceneSessionList, accessibilityInfo);
    EXPECT_EQ(accessibilityInfo.size(), 1);

    EXPECT_EQ(accessibilityInfo.at(0)->bundleName_, "accessibilityNotifyTesterBundleName");
    EXPECT_EQ(sceneSessionList.at(0)->GetSessionInfo().bundleName_, "accessibilityNotifyTesterBundleName");
    EXPECT_EQ(accessibilityInfo.at(0)->bundleName_, sceneSessionList.at(0)->GetSessionInfo().bundleName_);
}

/**
 * @tc.name: AccessibilityFillFilterBundleName
 * @tc.desc: SceneSesionManager fill filter bundle name to accessibilityInfo;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, AccessibilityFillFilterBundleName, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SCBGestureTopBar";
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";

    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    SetVisibleForAccessibility(sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    std::vector<sptr<SceneSession>> sceneSessionList;
    ssm_->GetAllSceneSessionForAccessibility(sceneSessionList);
    EXPECT_EQ(sceneSessionList.size(), 0);

    std::vector<sptr<AccessibilityWindowInfo>> accessibilityInfo;
    ssm_->FillAccessibilityInfo(sceneSessionList, accessibilityInfo);
    EXPECT_EQ(accessibilityInfo.size(), 0);
}

/**
 * @tc.name: AccessibilityFillEmptyHotAreas
 * @tc.desc: SceneSesionManager fill empty hot areas to accessibilityInfo;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, AccessibilityFillEmptyHotAreas, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "accessibilityNotifyTesterBundleName";
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";

    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    SetVisibleForAccessibility(sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    std::vector<sptr<SceneSession>> sceneSessionList;
    std::vector<sptr<AccessibilityWindowInfo>> accessibilityInfo;

    ssm_->GetAllSceneSessionForAccessibility(sceneSessionList);
    ssm_->FillAccessibilityInfo(sceneSessionList, accessibilityInfo);
    EXPECT_EQ(accessibilityInfo.size(), 1);

    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.size(), sceneSessionList.at(0)->GetTouchHotAreas().size());
    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.size(), 0);
}

/**
 * @tc.name: AccessibilityFillOneHotAreas
 * @tc.desc: SceneSesionManager fill one hot areas to accessibilityInfo;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, AccessibilityFillOneHotAreas, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "accessibilityNotifyTesterBundleName";
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";

    Rect rect = { 100, 200, 100, 200 };
    std::vector<Rect> hotAreas;
    hotAreas.push_back(rect);
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetTouchHotAreas(hotAreas);
    SetVisibleForAccessibility(sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    std::vector<sptr<SceneSession>> sceneSessionList;
    std::vector<sptr<AccessibilityWindowInfo>> accessibilityInfo;

    ssm_->GetAllSceneSessionForAccessibility(sceneSessionList);
    ssm_->FillAccessibilityInfo(sceneSessionList, accessibilityInfo);
    EXPECT_EQ(accessibilityInfo.size(), 1);

    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.size(), sceneSessionList.at(0)->GetTouchHotAreas().size());
    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.size(), 1);

    EXPECT_EQ(rect.posX_, sceneSessionList.at(0)->GetTouchHotAreas().at(0).posX_);
    EXPECT_EQ(rect.posY_, sceneSessionList.at(0)->GetTouchHotAreas().at(0).posY_);
    EXPECT_EQ(rect.width_, sceneSessionList.at(0)->GetTouchHotAreas().at(0).width_);
    EXPECT_EQ(rect.height_, sceneSessionList.at(0)->GetTouchHotAreas().at(0).height_);

    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.at(0).posX_, rect.posX_);
    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.at(0).posY_, rect.posY_);
    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.at(0).width_, rect.width_);
    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.at(0).height_, rect.height_);
}

/**
 * @tc.name: AccessibilityFillTwoHotAreas
 * @tc.desc: SceneSesionManager fill two hot areas to accessibilityInfo;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, AccessibilityFillTwoHotAreas, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "accessibilityNotifyTesterBundleName";
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    std::vector<Rect> hotAreas;
    Rect rectFitst = { 100, 200, 100, 200 };
    Rect rectSecond = { 50, 50, 20, 30 };
    hotAreas.push_back(rectFitst);
    hotAreas.push_back(rectSecond);
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetTouchHotAreas(hotAreas);
    SetVisibleForAccessibility(sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    std::vector<sptr<SceneSession>> sceneSessionList;
    std::vector<sptr<AccessibilityWindowInfo>> accessibilityInfo;

    ssm_->GetAllSceneSessionForAccessibility(sceneSessionList);
    ssm_->FillAccessibilityInfo(sceneSessionList, accessibilityInfo);
    EXPECT_EQ(accessibilityInfo.size(), 1);

    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.size(), sceneSessionList.at(0)->GetTouchHotAreas().size());
    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.size(), 2);

    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.at(0).posX_, rectFitst.posX_);
    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.at(0).posY_, rectFitst.posY_);
    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.at(0).width_, rectFitst.width_);
    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.at(0).height_, rectFitst.height_);

    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.at(1).posX_, rectSecond.posX_);
    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.at(1).posY_, rectSecond.posY_);
    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.at(1).width_, rectSecond.width_);
    EXPECT_EQ(accessibilityInfo.at(0)->touchHotAreas_.at(1).height_, rectSecond.height_);
}

/**
 * @tc.name: AccessibilityFilterEmptySceneSessionList
 * @tc.desc: SceneSesionManager filter empty scene session list;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, AccessibilityFilterEmptySceneSessionList, TestSize.Level1)
{
    std::vector<sptr<SceneSession>> sceneSessionList;

    ssm_->FilterSceneSessionCovered(sceneSessionList);
    EXPECT_EQ(sceneSessionList.size(), 0);
}

/**
 * @tc.name: AccessibilityFilterOneWindow
 * @tc.desc: SceneSesionManager filter one window;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, AccessibilityFilterOneWindow, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "accessibilityNotifyTesterBundleName";
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";

    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->SetSessionRect({ 100, 100, 200, 200 });
    SetVisibleForAccessibility(sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    std::vector<sptr<SceneSession>> sceneSessionList;
    std::vector<sptr<AccessibilityWindowInfo>> accessibilityInfo;
    ssm_->GetAllSceneSessionForAccessibility(sceneSessionList);
    ssm_->FilterSceneSessionCovered(sceneSessionList);
    ssm_->FillAccessibilityInfo(sceneSessionList, accessibilityInfo);
    EXPECT_EQ(accessibilityInfo.size(), 1);
}

/**
 * @tc.name: AccessibilityFilterTwoWindowNotCovered
 * @tc.desc: SceneSesionManager filter two windows that not covered each other;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, AccessibilityFilterTwoWindowNotCovered, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "accessibilityNotifyTesterBundleName";
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";

    sptr<SceneSession> sceneSessionFirst = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSessionFirst, nullptr);
    sceneSessionFirst->SetSessionRect({ 0, 0, 200, 200 });
    SetVisibleForAccessibility(sceneSessionFirst);
    ssm_->sceneSessionMap_.insert({ sceneSessionFirst->GetPersistentId(), sceneSessionFirst });

    sptr<SceneSession> sceneSessionSecond = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSessionSecond, nullptr);
    sceneSessionSecond->SetSessionRect({ 300, 300, 200, 200 });
    SetVisibleForAccessibility(sceneSessionSecond);
    ssm_->sceneSessionMap_.insert({ sceneSessionSecond->GetPersistentId(), sceneSessionSecond });

    std::vector<sptr<SceneSession>> sceneSessionList;
    std::vector<sptr<AccessibilityWindowInfo>> accessibilityInfo;
    ssm_->GetAllSceneSessionForAccessibility(sceneSessionList);
    ssm_->FilterSceneSessionCovered(sceneSessionList);
    ssm_->FillAccessibilityInfo(sceneSessionList, accessibilityInfo);
    EXPECT_EQ(accessibilityInfo.size(), 2);
}

/**
 * @tc.name: AccessibilityFilterTwoWindowCovered
 * @tc.desc: SceneSesionManager filter two windows that covered each other;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, AccessibilityFilterTwoWindowCovered, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "accessibilityNotifyTesterBundleName";
    sessionInfo.abilityName_ = "accessibilityNotifyTesterAbilityName";

    sptr<SceneSession> sceneSessionFirst = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSessionFirst, nullptr);
    sceneSessionFirst->SetSessionRect({ 0, 0, 200, 200 });
    SetVisibleForAccessibility(sceneSessionFirst);
    sceneSessionFirst->SetZOrder(20);
    ssm_->sceneSessionMap_.insert({ sceneSessionFirst->GetPersistentId(), sceneSessionFirst });

    sptr<SceneSession> sceneSessionSecond = ssm_->CreateSceneSession(sessionInfo, nullptr);
    ASSERT_NE(sceneSessionSecond, nullptr);
    sceneSessionSecond->SetSessionRect({ 50, 50, 50, 50 });
    SetVisibleForAccessibility(sceneSessionSecond);
    sceneSessionSecond->SetZOrder(10);
    ssm_->sceneSessionMap_.insert({ sceneSessionSecond->GetPersistentId(), sceneSessionSecond });

    std::vector<sptr<SceneSession>> sceneSessionList;
    std::vector<sptr<AccessibilityWindowInfo>> accessibilityInfo;
    ssm_->GetAllSceneSessionForAccessibility(sceneSessionList);
    ssm_->FilterSceneSessionCovered(sceneSessionList);
    ssm_->FillAccessibilityInfo(sceneSessionList, accessibilityInfo);
    EXPECT_EQ(accessibilityInfo.size(), 1);
}

/**
 * @tc.name: GetMainWindowInfos
 * @tc.desc: SceneSesionManager get topN main window infos;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetMainWindowInfos, TestSize.Level1)
{
    int32_t topNum = 1024;
    std::vector<MainWindowInfo> topNInfos;
    auto result = ssm_->GetMainWindowInfos(topNum, topNInfos);
    EXPECT_EQ(result, WMError::WM_OK);

    topNum = 0;
    ssm_->GetMainWindowInfos(topNum, topNInfos);

    topNum = 1000;
    MainWindowInfo info;
    topNInfos.push_back(info);
    ssm_->GetMainWindowInfos(topNum, topNInfos);
}

/**
 * @tc.name: GetAllWindowVisibilityInfos
 * @tc.desc: SceneSesionManager get all window visibility infos;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetAllWindowVisibilityInfos, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    std::vector<std::pair<int32_t, uint32_t>> windowVisibilityInfos;
    ssm_->GetAllWindowVisibilityInfos(windowVisibilityInfos);
    EXPECT_NE(windowVisibilityInfos.size(), 0);
}

/**
 * @tc.name: TestNotifyEnterRecentTask
 * @tc.desc: Test whether the enterRecent_ is set correctly;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, TestNotifyEnterRecentTask, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneSessionManagerTest: TestNotifyEnterRecentTask start";
    sptr<SceneSessionManager> sceneSessionManager = sptr<SceneSessionManager>::MakeSptr();
    ASSERT_NE(nullptr, sceneSessionManager);

    EXPECT_EQ(sceneSessionManager->NotifyEnterRecentTask(true), WSError::WS_OK);
    EXPECT_EQ(sceneSessionManager->enterRecent_.load(), true);
}

/**
 * @tc.name: TestNotifySCBRecentStateChange(bool isRecent)
 * @tc.desc: Test whether the isRecent_ is set correctly;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, TestNotifySCBRecentStateChange, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneSessionManagerTest: TestNotifySCBRecentStateChange start";
    sptr<SceneSessionManager> sceneSessionManager = sptr<SceneSessionManager>::MakeSptr();
    ASSERT_NE(nullptr, sceneSessionManager);

    sceneSessionManager->NotifySCBRecentStateChange(true);
    EXPECT_EQ(sceneSessionManager->scbIsRecent_.load(), true);
}

/**
 * @tc.name: TestIsEnablePiPCreate
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, TestIsEnablePiPCreate, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneSessionManagerTest: TestIsEnablePiPCreate start";
    ASSERT_TRUE(!ssm_->IsEnablePiPCreate(nullptr));
    ssm_->isScreenLocked_ = true;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_TRUE(!ssm_->IsEnablePiPCreate(property));

    ssm_->isScreenLocked_ = false;
    Rect reqRect = { 0, 0, 0, 0 };
    property->SetRequestRect(reqRect);
    ASSERT_TRUE(!ssm_->IsEnablePiPCreate(property));

    reqRect = { 0, 0, 10, 0 };
    property->SetRequestRect(reqRect);
    ASSERT_TRUE(!ssm_->IsEnablePiPCreate(property));

    reqRect = { 0, 0, 10, 10 };
    property->SetRequestRect(reqRect);
    PiPTemplateInfo info = {};
    property->SetPiPTemplateInfo(info);
    SessionInfo info1;
    info1.abilityName_ = "test1";
    info1.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    property->SetWindowMode(WindowMode::WINDOW_MODE_PIP);
    sceneSession->pipTemplateInfo_ = {};
    sceneSession->pipTemplateInfo_.priority = 100;
    ssm_->sceneSessionMap_.insert({ 0, sceneSession });
    ASSERT_TRUE(!ssm_->IsEnablePiPCreate(property));
    ssm_->sceneSessionMap_.clear();
    ASSERT_TRUE(!ssm_->IsEnablePiPCreate(property));

    property->SetParentPersistentId(100);
    ASSERT_TRUE(!ssm_->IsEnablePiPCreate(property));

    ssm_->sceneSessionMap_.insert({ 100, sceneSession });
    ASSERT_TRUE(!ssm_->IsEnablePiPCreate(property));

    ssm_->sceneSessionMap_.clear();
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    ssm_->sceneSessionMap_.insert({ 100, sceneSession });
    ASSERT_TRUE(ssm_->IsEnablePiPCreate(property));
}

/**
 * @tc.name: TestGetScreenName
 * @tc.desc: Test get screenName;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, TestGetScreenName, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneSessionManagerTest: TestGetScreenName start";
    int32_t persistentId = 2000;
    EXPECT_EQ(ssm_->GetScreenName(persistentId), "");

    SessionInfo info1;
    info1.abilityName_ = "test1";
    info1.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ssm_->sceneSessionMap_[persistentId] = sceneSession;
    EXPECT_EQ(ssm_->GetScreenName(persistentId), "");

    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    EXPECT_EQ(ssm_->GetScreenName(persistentId), "");

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sceneSession->SetSessionProperty(property);
    EXPECT_EQ(ssm_->GetScreenName(persistentId), "");

    property->SetDisplayId(-1ULL);
    sceneSession->SetSessionProperty(property);
    ssm_->sceneSessionMap_[persistentId] = sceneSession;
    EXPECT_EQ(ssm_->GetScreenName(persistentId), "");

    uint64_t displayId = 1000;
    property->SetDisplayId(displayId);
    sceneSession->SetSessionProperty(property);
    ssm_->sceneSessionMap_[persistentId] = sceneSession;
    EXPECT_EQ(ssm_->GetScreenName(persistentId), "");

    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert({displayId, screenSession});
    EXPECT_EQ(ssm_->GetScreenName(persistentId), "UNKNOWN");

    screenSession->SetName("SuperLauncher");
    EXPECT_EQ(ssm_->GetScreenName(persistentId), "SuperLauncher");
    ssm_->sceneSessionMap_.erase(2000);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.erase(1000);
}

/**
 * @tc.name: TestIsEnablePiPCreateOnSuperLauncher
 * @tc.desc: Test if pip window can be created;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, TestIsEnablePiPCreateOnSuperLauncher, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneSessionManagerTest: TestIsEnablePiPCreateOnSuperLauncher start";
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    Rect reqRect = { 0, 0, 10, 10 };
    property->SetRequestRect(reqRect);
    property->SetParentPersistentId(100);

    SessionInfo info1;
    info1.abilityName_ = "test1";
    info1.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info1, nullptr);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    ssm_->sceneSessionMap_.insert({100, sceneSession});

    uint64_t displayId = 2000;
    property->SetDisplayId(displayId);
    sceneSession->SetSessionProperty(property);
    ssm_->sceneSessionMap_.insert({100, sceneSession});
    sptr<ScreenSession> screenSession = sptr<ScreenSession>::MakeSptr();
    screenSession->SetName("SuperLauncher");
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert({displayId, screenSession});
    ssm_->isScreenLocked_ = true;
    ASSERT_TRUE(ssm_->IsEnablePiPCreate(property));
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.erase(2000);
}

/**
 * @tc.name: TestIsPiPForbidden
 * @tc.desc: Test if pip window is forbidden to use;
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, TestIsPiPForbidden, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SceneSessionManagerTest: TestIsPiPForbidden start";
    int32_t persistentId = 1001;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetParentPersistentId(persistentId);
    ASSERT_TRUE(!ssm_->IsPiPForbidden(property, WindowType::WINDOW_TYPE_PIP));

    SessionInfo sessionInfo;
    sessionInfo.persistentId_ = persistentId;
    sessionInfo.bundleName_ = "test1";
    sessionInfo.abilityName_ = "test2";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    property->SetDisplayId(-1ULL);
    sceneSession->SetSessionProperty(property);
    ssm_->sceneSessionMap_.insert({ persistentId, sceneSession });
    ASSERT_TRUE(!ssm_->IsPiPForbidden(property, WindowType::WINDOW_TYPE_PIP));

    uint64_t displayId = 1001;
    property->SetDisplayId(displayId);
    sceneSession->SetSessionProperty(property);
    ssm_->sceneSessionMap_[persistentId] = sceneSession;
    sptr<ScreenSession> screenSession = new ScreenSession();
    screenSession->SetName("HiCar");
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert({ displayId, screenSession });
    ASSERT_TRUE(ssm_->IsPiPForbidden(property, WindowType::WINDOW_TYPE_PIP));
    ASSERT_TRUE(!ssm_->IsPiPForbidden(property, WindowType::WINDOW_TYPE_FLOAT));

    screenSessionForPad->SetName("CustomScbScreen");
    ssm_->SetPipEnableByScreenId(displayIdForPad, false);
    ASSERT_TRUE(ssm_->IsPiPForbidden(property, WindowType::WINDOW_TYPE_PIP));
    ASSERT_TRUE(!ssm_->IsPiPForbidden(property, WindowType::WINDOW_TYPE_FB));
}

/**
 * @tc.name: GetAllMainWindowInfos
 * @tc.desc: GetAllMainWindowInfos
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetAllMainWindowInfos, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_SUB_WINDOW_BASE);
    info.persistentId_ = 100;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    std::vector<MainWindowInfo> infos;
    WMError result = ssm_->GetAllMainWindowInfos(infos);
    EXPECT_EQ(result, WMError::WM_OK);
    ssm_->sceneSessionMap_.erase(sceneSession->GetPersistentId());
}

/**
 * @tc.name: GetAllMainWindowInfos001
 * @tc.desc: SceneSessionManager get all main window infos.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetAllMainWindowInfos001, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test1";
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_WINDOW_BASE);
    info.persistentId_ = 1;
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    AppExecFwk::ApplicationInfo applicationInfo;
    applicationInfo.bundleType = AppExecFwk::BundleType::ATOMIC_SERVICE;
    abilityInfo->applicationInfo = applicationInfo;
    info.abilityInfo = abilityInfo;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    std::vector<MainWindowInfo> infos;
    WMError result = ssm_->GetAllMainWindowInfos(infos);
    EXPECT_EQ(result, WMError::WM_OK);
    ssm_->sceneSessionMap_.erase(sceneSession->GetPersistentId());
}

/**
 * @tc.name: GetAllMainWindowInfos002
 * @tc.desc: SceneSessionManager get all main window infos, input params are not empty.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetAllMainWindowInfos002, TestSize.Level1)
{
    std::vector<MainWindowInfo> infos;
    MainWindowInfo info;
    info.pid_ = 1000;
    info.bundleName_ = "test";
    infos.push_back(info);
    WMError result = ssm_->GetAllMainWindowInfos(infos);
    EXPECT_EQ(result, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: GetUnreliableWindowInfo01
 * @tc.desc: SceneSesionManager get unreliable window info, windowId correct
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetUnreliableWindowInfo01, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    int32_t windowId = sceneSession->GetPersistentId();
    std::vector<sptr<UnreliableWindowInfo>> infos;
    WMError result = ssm_->GetUnreliableWindowInfo(windowId, infos);
    EXPECT_EQ(WMError::WM_OK, result);
    EXPECT_EQ(1, infos.size());
}

/**
 * @tc.name: GetUnreliableWindowInfo02
 * @tc.desc: SceneSesionManager get unreliable window info, toast window
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetUnreliableWindowInfo02, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.windowType_ = 2107;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_TOAST);
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, property);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetRSVisible(true);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    int32_t windowId = 0;
    std::vector<sptr<UnreliableWindowInfo>> infos;
    WMError result = ssm_->GetUnreliableWindowInfo(windowId, infos);
    EXPECT_EQ(WMError::WM_OK, result);
    EXPECT_EQ(1, infos.size());
}

/**
 * @tc.name: GetUnreliableWindowInfo03
 * @tc.desc: SceneSesionManager get unreliable window info, app sub window
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetUnreliableWindowInfo03, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.windowType_ = 1000;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, property);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    SessionInfo info2;
    info2.windowType_ = 1001;
    sptr<WindowSessionProperty> property2 = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property2);
    property2->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    property2->SetParentId(sceneSession->GetPersistentId());
    sptr<SceneSession> sceneSession2 = ssm_->CreateSceneSession(info2, property2);
    ASSERT_NE(nullptr, sceneSession2);
    sceneSession2->SetRSVisible(true);
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });

    int32_t windowId = 0;
    std::vector<sptr<UnreliableWindowInfo>> infos;
    WMError result = ssm_->GetUnreliableWindowInfo(windowId, infos);
    EXPECT_EQ(WMError::WM_OK, result);
    EXPECT_EQ(1, infos.size());
}

/**
 * @tc.name: GetUnreliableWindowInfo04
 * @tc.desc: SceneSesionManager get unreliable window info, input method window
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetUnreliableWindowInfo04, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.windowType_ = 2105;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, property);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetRSVisible(true);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    int32_t windowId = 0;
    std::vector<sptr<UnreliableWindowInfo>> infos;
    WMError result = ssm_->GetUnreliableWindowInfo(windowId, infos);
    EXPECT_EQ(WMError::WM_OK, result);
    EXPECT_EQ(1, infos.size());
}

/**
 * @tc.name: GetUnreliableWindowInfo05
 * @tc.desc: SceneSesionManager get unreliable window info, not correct window type, not visible
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetUnreliableWindowInfo05, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.windowType_ = 2122;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, property);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetRSVisible(true);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ssm_->sceneSessionMap_.insert({ 0, nullptr });

    int32_t windowId = 0;
    std::vector<sptr<UnreliableWindowInfo>> infos;
    WMError result = ssm_->GetUnreliableWindowInfo(windowId, infos);
    EXPECT_EQ(WMError::WM_OK, result);
    sceneSession->SetRSVisible(false);
    result = ssm_->GetUnreliableWindowInfo(windowId, infos);
    EXPECT_EQ(WMError::WM_OK, result);
    EXPECT_EQ(0, infos.size());
}

/**
 * @tc.name: GetUnreliableWindowInfo06
 * @tc.desc: SceneSesionManager satisfy FillUnreliableWindowInfo branches coverage
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetUnreliableWindowInfo06, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info1;
    info1.bundleName_ = "SCBGestureBack";
    sptr<SceneSession> sceneSession1 = ssm_->CreateSceneSession(info1, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });

    SessionInfo info2;
    info2.bundleName_ = "SCBGestureNavBar";
    sptr<SceneSession> sceneSession2 = ssm_->CreateSceneSession(info2, nullptr);
    ASSERT_NE(nullptr, sceneSession2);
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });

    SessionInfo info3;
    info3.bundleName_ = "SCBGestureTopBar";
    sptr<SceneSession> sceneSession3 = ssm_->CreateSceneSession(info3, nullptr);
    ASSERT_NE(nullptr, sceneSession3);
    ssm_->sceneSessionMap_.insert({ sceneSession3->GetPersistentId(), sceneSession3 });

    std::vector<sptr<UnreliableWindowInfo>> infos;
    ssm_->GetUnreliableWindowInfo(sceneSession1->GetPersistentId(), infos);
    ssm_->GetUnreliableWindowInfo(sceneSession2->GetPersistentId(), infos);
    ssm_->GetUnreliableWindowInfo(sceneSession3->GetPersistentId(), infos);
    EXPECT_EQ(0, infos.size());
}

/**
 * @tc.name: GetUnreliableWindowInfo07
 * @tc.desc: system touchable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, GetUnreliableWindowInfo07, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.windowType_ = 2122;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, property);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetRSVisible(true);
    sceneSession->SetSystemTouchable(false);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    ssm_->sceneSessionMap_.insert({ 0, nullptr });

    int32_t windowId = 0;
    std::vector<sptr<UnreliableWindowInfo>> infos;
    WMError result = ssm_->GetUnreliableWindowInfo(windowId, infos);
    EXPECT_EQ(WMError::WM_OK, result);
    sceneSession->SetRSVisible(false);
    result = ssm_->GetUnreliableWindowInfo(windowId, infos);
    EXPECT_EQ(WMError::WM_OK, result);
    EXPECT_EQ(0, infos.size());
}

/**
 * @tc.name: SkipSnapshotForAppProcess
 * @tc.desc: add or cancel snapshot skip for app process
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, SkipSnapshotForAppProcess, TestSize.Level1)
{
    int32_t pid = 1000;
    bool skip = true;
    auto result = ssm_->SkipSnapshotForAppProcess(pid, skip);
    ASSERT_EQ(result, WMError::WM_OK);
    usleep(WAIT_SYNC_FOR_SNAPSHOT_SKIP_IN_NS);
    ASSERT_NE(ssm_->snapshotSkipPidSet_.find(pid), ssm_->snapshotSkipPidSet_.end());

    skip = false;
    result = ssm_->SkipSnapshotForAppProcess(pid, skip);
    ASSERT_EQ(result, WMError::WM_OK);
    usleep(WAIT_SYNC_FOR_SNAPSHOT_SKIP_IN_NS);
    ASSERT_EQ(ssm_->snapshotSkipPidSet_.find(pid), ssm_->snapshotSkipPidSet_.end());

    SessionInfo info;
    sptr<SceneSession> sceneSession1 = ssm_->CreateSceneSession(info, nullptr);
    sptr<SceneSession> sceneSession2 = ssm_->CreateSceneSession(info, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    ASSERT_NE(nullptr, sceneSession2);
    sceneSession1->SetCallingPid(1000);
    sceneSession2->SetCallingPid(1001);
    ssm_->sceneSessionMap_.insert({ sceneSession1->GetPersistentId(), sceneSession1 });
    ssm_->sceneSessionMap_.insert({ sceneSession2->GetPersistentId(), sceneSession2 });
    ssm_->sceneSessionMap_.insert({ -1, nullptr });
    skip = true;
    result = ssm_->SkipSnapshotForAppProcess(pid, skip);
    usleep(WAIT_SYNC_FOR_SNAPSHOT_SKIP_IN_NS);
    EXPECT_EQ(result, WMError::WM_OK);
    skip = false;
    result = ssm_->SkipSnapshotForAppProcess(pid, skip);
    EXPECT_EQ(result, WMError::WM_OK);
    ssm_->sceneSessionMap_.erase(sceneSession1->GetPersistentId());
    ssm_->sceneSessionMap_.erase(sceneSession2->GetPersistentId());
    ssm_->sceneSessionMap_.erase(-1);
    usleep(WAIT_SYNC_FOR_TEST_END_IN_NS);
}

/**
 * @tc.name: NotifySessionTransferToTargetScreenEvent001
 * @tc.desc: NotifySessionTransferToTargetScreenEventTest persistentId is invalid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, NotifySessionTransferToTargetScreenEvent001, TestSize.Level1)
{
    g_logMsg.clear();
    int32_t persistentId = -1;
    uint32_t resultCode = 1;
    uint64_t fromScreenId = 1;
    uint64_t toScreenId = 1;
    ssm_->NotifySessionTransferToTargetScreenEvent(persistentId, resultCode, fromScreenId, toScreenId);
    EXPECT_TRUE(g_logMsg.find("sceneSession is nullptr") != std::string::npos);
}

/**
 * @tc.name: NotifySessionTransferToTargetScreenEvent002
 * @tc.desc: NotifySessionTransferToTargetScreenEventTest persistentId is valid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, NotifySessionTransferToTargetScreenEvent002, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    info.persistentId_ = 9527;
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_WINDOW_BASE);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    g_logMsg.clear();
    ssm_->NotifySessionTransferToTargetScreenEvent(sceneSession->GetPersistentId(), 1, 1, 2);
    EXPECT_FALSE(g_logMsg.find("sceneSession is nullptr") != std::string::npos);
}

/**
 *@tc.name: UpdateAppBoundSystemTrayStatus001
 *@tc.desc: UpdateAppBoundSystemTrayStatusTest persistentId is invalid
 *@tc.type: FUNC
*/
HWTEST_F(SceneSessionManagerTest, UpdateAppBoundSystemTrayStatus001, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    int32_t pid = 1000; // 初始 pid
    uint32_t callingToken = 2000; // 初始 callingToken
    std::string instanceKey = "app_instance_0";
    // 插入 50 条记录，pid 和 callingToken 递增
    for (int i = 0; i < 50; ++i) {
        // 生成 key: callingToken + "-" + instanceKey
        std::string key = std::to_string(callingToken) + "-" + instanceKey;
        // 插入当前 pid 到对应的 unordered_set 中
        ssm_->appsWithBoundSystemTrayMap_[key].insert(pid);
        // pid 和 callingToken 自增
        pid++;
        callingToken++;
    }

    // pid=1049 callingToken=2049
    ssm_->UpdateAppBoundSystemTrayStatus("2049-app_instance_0", 1049, true);
    ASSERT_EQ(ssm_->appsWithBoundSystemTrayMap_.size() >= 50, true);
    EXPECT_TRUE(g_logMsg.find("App bound tray map size exceeds") != std::string::npos);

    ssm_->appsWithBoundSystemTrayMap_.erase("2049-app_instance_0");
    ssm_->UpdateAppBoundSystemTrayStatus("2049-app_instance_0", 1049, false);
    ssm_->appsWithBoundSystemTrayMap_.clear();

    ssm_->appsWithBoundSystemTrayMap_["2000-app_instance_0"].insert(1000);
    ssm_->UpdateAppBoundSystemTrayStatus("2000-app_instance_0", 1000, false);
    ssm_->appsWithBoundSystemTrayMap_.clear();

    LOG_SetCallback(nullptr);
}

/**
 *@tc.name: IsAppBoundSystemTray001
 *@tc.desc: IsAppBoundSystemTrayTest persistentId is invalid
 *@tc.type: FUNC
*/
HWTEST_F(SceneSessionManagerTest, IsAppBoundSystemTray001, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);

    ssm_->UpdateAppBoundSystemTrayStatus("2000-app_instance_0", 1000, true);
    ASSERT_EQ(ssm_->IsAppBoundSystemTray(1000, 2000, "app_instance_0"), true);

    ASSERT_EQ(ssm_->IsAppBoundSystemTray(1001, 2001, "app_instance_0"), false);
    ssm_->appsWithBoundSystemTrayMap_.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetSupportRotationRegisteredListener
 * @tc.desc: SetSupportRotationRegisteredListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, SetSupportRotationRegisteredListener, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    info.persistentId_ = 9527;
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_WINDOW_BASE);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    g_logMsg.clear();
    NotifySupportRotationRegisteredFunc func = [](){};
    ssm_->SetSupportRotationRegisteredListener(std::move(func));
    EXPECT_TRUE(ssm_->supportRotationRegisteredListener_ != nullptr);
}

/**
 * @tc.name: NotifySupportRotationRegistered
 * @tc.desc: NotifySupportRotationRegistered
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest, NotifySupportRotationRegistered, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "test";
    info.bundleName_ = "test";
    info.persistentId_ = 9527;
    info.windowType_ = static_cast<uint32_t>(WindowType::APP_WINDOW_BASE);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });

    g_logMsg.clear();
    WMError ret = WMError::WM_OK;
    ret = ssm_->NotifySupportRotationRegistered();
    EXPECT_EQ(ret, WMError::WM_OK);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
