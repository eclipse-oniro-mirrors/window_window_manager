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

#include <bundlemgr/launcher_service.h>
#include "interfaces/include/ws_common.h"
#include "iremote_object_mocker.h"
#include "mock/mock_accesstoken_kit.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "session_manager.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/main_session.h"
#include "mock/mock_ibundle_mgr.h"
#include "common/include/task_scheduler.h"
#include "session/host/include/multi_instance_manager.h"
#include "test/mock/mock_session_stage.h"
#include "test/mock/mock_window_event_channel.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const std::string BUNDLE_NAME = "bundleName";
const int32_t USER_ID{ 100 };
const int32_t SLEEP_TIME{ 10000 };
} // namespace
class SceneSessionManagerTest11 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    sptr<SceneSession> CreateSceneSession(const std::string& bundleName, WindowType windowType);

    static sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
    sptr<SceneSession> GetSceneSession(const std::string& instanceKey = "");
    void Init(AppExecFwk::MultiAppModeType modeType, uint32_t maxCount);
    std::shared_ptr<TaskScheduler> GetTaskScheduler();
};

sptr<SceneSessionManager> SceneSessionManagerTest11::ssm_ = nullptr;

void SceneSessionManagerTest11::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
}

void SceneSessionManagerTest11::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerTest11::SetUp() {}

void SceneSessionManagerTest11::TearDown()
{
    sptr<IBundleMgrMocker> bundleMgrMocker = sptr<IBundleMgrMocker>::MakeSptr();
    EXPECT_CALL(*bundleMgrMocker, GetApplicationInfo(_, _, _, _)).WillOnce(Return(false));
    MultiInstanceManager::GetInstance().Init(bundleMgrMocker, GetTaskScheduler());
    ssm_->RefreshAppInfo(BUNDLE_NAME);
    usleep(SLEEP_TIME);
    MockAccesstokenKit::ChangeMockStateToInit();
}

sptr<SceneSession> SceneSessionManagerTest11::GetSceneSession(const std::string& instanceKey)
{
    SessionInfo info;
    info.bundleName_ = BUNDLE_NAME;
    info.appInstanceKey_ = instanceKey;
    info.isNewAppInstance_ = true;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    return sceneSession;
}

void SceneSessionManagerTest11::Init(AppExecFwk::MultiAppModeType modeType, uint32_t maxCount)
{
    sptr<IBundleMgrMocker> bundleMgrMocker = sptr<IBundleMgrMocker>::MakeSptr();
    EXPECT_CALL(*bundleMgrMocker, GetApplicationInfosV9(_, _, _)).WillOnce([modeType, maxCount](
        int32_t flags, int32_t userId, std::vector<AppExecFwk::ApplicationInfo>& appInfos) {
        AppExecFwk::ApplicationInfo appInfo;
        appInfo.bundleName = BUNDLE_NAME;
        appInfo.multiAppMode.multiAppModeType = modeType;
        appInfo.multiAppMode.maxCount = maxCount;
        appInfos.push_back(appInfo);
        return ERR_OK;
    });
    MultiInstanceManager::GetInstance().Init(bundleMgrMocker, GetTaskScheduler());
    MultiInstanceManager::GetInstance().SetCurrentUserId(USER_ID);
    usleep(SLEEP_TIME);
}

std::shared_ptr<TaskScheduler> SceneSessionManagerTest11::GetTaskScheduler()
{
    std::string threadName = "threadName";
    std::shared_ptr<TaskScheduler> taskScheduler = std::make_shared<TaskScheduler>(threadName);
    return taskScheduler;
}

sptr<SceneSession> SceneSessionManagerTest11::CreateSceneSession(const std::string& bundleName, WindowType windowType)
{
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = bundleName;

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(windowType);
    property->SetWindowName(bundleName);

    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_ = property;
    return sceneSession;
}

namespace {

std::string g_logMsg;
void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char* tag,
    const char* msg)
{
    g_logMsg += msg;
}

/**
 * @tc.name: GetMainWindowStatesByPid
 * @tc.desc: SceneSesionManager get main window states by pid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetMainWindowStatesByPid, TestSize.Level1)
{
    int32_t pid = 100;
    std::vector<MainWindowState> windowStates;
    WSError result = ssm_->GetMainWindowStatesByPid(pid, windowStates);
    EXPECT_EQ(result, WSError::WS_OK);
}

/**
 * @tc.name: GetMainWindowStatesByPid02
 * @tc.desc: SceneSesionManager get main window states by pid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetMainWindowStatesByPid02, TestSize.Level1)
{
    int32_t invalidPid = -1;
    std::vector<MainWindowState> windowStates;
    WSError result = ssm_->GetMainWindowStatesByPid(invalidPid, windowStates);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: GetMainWindowStatesByPid03
 * @tc.desc: SceneSesionManager get main window states by pid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetMainWindowStatesByPid03, TestSize.Level1)
{
    SessionState sessionState = SessionState::STATE_FOREGROUND;
    bool isVisible = true;
    bool isForegroundInteractive = true;
    bool isPcOrPadEnableActivation = true;
    int32_t callingPid = 1001;
    SessionInfo sessionInfo;
    int32_t persistentId = 1005;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    EXPECT_NE(sceneSession, nullptr);
    sceneSession->SetSessionState(sessionState);
    sceneSession->SetRSVisible(isVisible);
    sceneSession->SetForegroundInteractiveStatus(isForegroundInteractive);
    sceneSession->GetSessionProperty()->SetIsPcAppInPad(isPcOrPadEnableActivation);
    sceneSession->SetCallingPid(callingPid);
    ssm_->sceneSessionMap_.insert({ persistentId, sceneSession });
    std::vector<MainWindowState> windowStates;
    WSError result = ssm_->GetMainWindowStatesByPid(callingPid, windowStates);
    EXPECT_EQ(result, WSError::WS_OK);
    EXPECT_EQ(windowStates.size(), 1);
    EXPECT_EQ(windowStates[0].state_, static_cast<int32_t>(sessionState));
    EXPECT_EQ(windowStates[0].isVisible_, isVisible);
    EXPECT_EQ(windowStates[0].isForegroundInteractive_, isForegroundInteractive);
    EXPECT_EQ(windowStates[0].isPcOrPadEnableActivation_, isPcOrPadEnableActivation);
}

/**
 * @tc.name: GetMaxInstanceCount
 * @tc.desc: test function : GetMaxInstanceCount
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetMaxInstanceCount, TestSize.Level1)
{
    AppExecFwk::MultiAppModeType modeType = AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    uint32_t maxCount = 5;
    Init(modeType, maxCount);
    ASSERT_EQ(ssm_->GetMaxInstanceCount(BUNDLE_NAME), maxCount);
}

/**
 * @tc.name: GetInstanceCount
 * @tc.desc: test function : GetInstanceCount
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetInstanceCount, TestSize.Level1)
{
    AppExecFwk::MultiAppModeType modeType = AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    uint32_t maxCount = 5;
    Init(modeType, maxCount);
    ASSERT_EQ(ssm_->GetInstanceCount(BUNDLE_NAME), 0);
    std::string instanceKey0 = "app_instance_0";
    sptr<SceneSession> sceneSession = GetSceneSession(instanceKey0);
    ASSERT_EQ(MultiInstanceManager::GetInstance().CreateNewInstanceKey(BUNDLE_NAME), instanceKey0);
    MultiInstanceManager::GetInstance().IncreaseInstanceKeyRefCount(sceneSession);
    ASSERT_EQ(ssm_->GetInstanceCount(BUNDLE_NAME), 1);
    MultiInstanceManager::GetInstance().DecreaseInstanceKeyRefCount(sceneSession);
    ASSERT_EQ(ssm_->GetInstanceCount(BUNDLE_NAME), 0);
}

/**
 * @tc.name: GetLastInstanceKey
 * @tc.desc: test function : GetLastInstanceKey
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetLastInstanceKey, TestSize.Level1)
{
    AppExecFwk::MultiAppModeType modeType = AppExecFwk::MultiAppModeType::MULTI_INSTANCE;
    uint32_t maxCount = 5;
    Init(modeType, maxCount);
    ASSERT_EQ(ssm_->GetLastInstanceKey(BUNDLE_NAME), "");
    std::string instanceKey0 = "app_instance_0";
    sptr<SceneSession> sceneSession = GetSceneSession(instanceKey0);
    ASSERT_EQ(MultiInstanceManager::GetInstance().CreateNewInstanceKey(BUNDLE_NAME), instanceKey0);
    MultiInstanceManager::GetInstance().IncreaseInstanceKeyRefCount(sceneSession);
    ASSERT_EQ(ssm_->GetLastInstanceKey(BUNDLE_NAME), instanceKey0);
    MultiInstanceManager::GetInstance().DecreaseInstanceKeyRefCount(sceneSession);
    ASSERT_EQ(ssm_->GetLastInstanceKey(BUNDLE_NAME), "");
}

/**
 * @tc.name: GetAbilityInfo
 * @tc.desc: SceneSesionManager test GetAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetAbilityInfo, TestSize.Level1)
{
    ssm_->bundleMgr_ = nullptr;
    std::string bundleName = "bundleName";
    std::string moduleName = "moduleName";
    std::string abilityName = "abilityName";
    int32_t userId = 100;
    SCBAbilityInfo scbAbilityInfo;
    WSError ret = ssm_->GetAbilityInfo(bundleName, moduleName, abilityName, userId, scbAbilityInfo);
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: GetAbilityInfo02
 * @tc.desc: SceneSesionManager test GetAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetAbilityInfo02, TestSize.Level1)
{
    sptr<IBundleMgrMocker> bundleMgrMocker = sptr<IBundleMgrMocker>::MakeSptr();
    EXPECT_CALL(*bundleMgrMocker, GetBundleInfoV9(_, _, _, _)).WillOnce(Return(1));
    ssm_->bundleMgr_ = bundleMgrMocker;
    std::string bundleName = "bundleName";
    std::string moduleName = "moduleName";
    std::string abilityName = "abilityName";
    int32_t userId = 100;
    SCBAbilityInfo scbAbilityInfo;
    WSError ret = ssm_->GetAbilityInfo(bundleName, moduleName, abilityName, userId, scbAbilityInfo);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: GetAbilityInfo03
 * @tc.desc: SceneSesionManager test GetAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetAbilityInfo03, TestSize.Level1)
{
    sptr<IBundleMgrMocker> bundleMgrMocker = sptr<IBundleMgrMocker>::MakeSptr();
    EXPECT_CALL(*bundleMgrMocker, GetBundleInfoV9(_, _, _, _))
        .WillOnce([](const std::string& bundleName, int32_t flags, AppExecFwk::BundleInfo& bundleInfo, int32_t userId) {
            bundleInfo.hapModuleInfos = {};
            return 0;
        });
    ssm_->bundleMgr_ = bundleMgrMocker;
    std::string bundleName = "bundleName";
    std::string moduleName = "moduleName";
    std::string abilityName = "abilityName";
    int32_t userId = 100;
    SCBAbilityInfo scbAbilityInfo;
    WSError ret = ssm_->GetAbilityInfo(bundleName, moduleName, abilityName, userId, scbAbilityInfo);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: GetAbilityInfo04
 * @tc.desc: SceneSesionManager test GetAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetAbilityInfo04, TestSize.Level1)
{
    sptr<IBundleMgrMocker> bundleMgrMocker = sptr<IBundleMgrMocker>::MakeSptr();
    EXPECT_CALL(*bundleMgrMocker, GetBundleInfoV9(_, _, _, _))
        .WillOnce([](const std::string& bundleName, int32_t flags, AppExecFwk::BundleInfo& bundleInfo, int32_t userId) {
            AppExecFwk::AbilityInfo abilityInfo;
            abilityInfo.moduleName = "moduleName";
            abilityInfo.name = "abilityName";
            AppExecFwk::HapModuleInfo hapModuleInfo;
            hapModuleInfo.abilityInfos = { abilityInfo };
            bundleInfo.hapModuleInfos = { hapModuleInfo };
            bundleInfo.applicationInfo.codePath = "testCodePath";
            return 0;
        });
    ssm_->bundleMgr_ = bundleMgrMocker;
    std::string bundleName = "bundleName";
    std::string moduleName = "moduleName";
    std::string abilityName = "abilityName";
    int32_t userId = 100;
    SCBAbilityInfo scbAbilityInfo;
    WSError ret = ssm_->GetAbilityInfo(bundleName, moduleName, abilityName, userId, scbAbilityInfo);
    ASSERT_EQ(ret, WSError::WS_OK);
    ASSERT_EQ(scbAbilityInfo.codePath_, "testCodePath");
}

/**
 * @tc.name: GetAbilityInfo05
 * @tc.desc: SceneSesionManager test GetAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetAbilityInfo05, TestSize.Level1)
{
    sptr<IBundleMgrMocker> bundleMgrMocker = sptr<IBundleMgrMocker>::MakeSptr();
    EXPECT_CALL(*bundleMgrMocker, GetBundleInfoV9(_, _, _, _))
        .WillOnce([](const std::string& bundleName, int32_t flags, AppExecFwk::BundleInfo& bundleInfo, int32_t userId) {
            AppExecFwk::AbilityInfo abilityInfo;
            abilityInfo.moduleName = "moduleName2";
            abilityInfo.name = "abilityName2";
            AppExecFwk::HapModuleInfo hapModuleInfo;
            hapModuleInfo.abilityInfos = { abilityInfo };
            bundleInfo.hapModuleInfos = { hapModuleInfo };
            return 0;
        });
    ssm_->bundleMgr_ = bundleMgrMocker;
    std::string bundleName = "bundleName";
    std::string moduleName = "moduleName";
    std::string abilityName = "abilityName";
    int32_t userId = 100;
    SCBAbilityInfo scbAbilityInfo;
    WSError ret = ssm_->GetAbilityInfo(bundleName, moduleName, abilityName, userId, scbAbilityInfo);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: LockSessionByAbilityInfo
 * @tc.desc: SceneSesionManager test LockSessionByAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, LockSessionByAbilityInfo, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    AbilityInfoBase abilityInfo;
    abilityInfo.bundleName = "LockSessionByAbilityInfoBundle";
    abilityInfo.moduleName = "LockSessionByAbilityInfoModule";
    abilityInfo.abilityName = "LockSessionByAbilityInfoAbility";
    abilityInfo.appIndex = 0;

    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    auto result = ssm_->LockSessionByAbilityInfo(abilityInfo, true);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, result);
}

/**
 * @tc.name: NotifyWatchGestureConsumeResult
 * @tc.desc: SceneSesionManager test NotifyWatchGestureConsumeResult
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, NotifyWatchGestureConsumeResult, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    int32_t keyCode = 0;
    bool isConsumed = true;
    ssm_->onWatchGestureConsumeResultFunc_ = [](int32_t keyCode, bool isConsumed) {};
    auto ret = ssm_->NotifyWatchGestureConsumeResult(keyCode, isConsumed);
    ASSERT_EQ(ret, WMError::WM_OK);

    ssm_->onWatchGestureConsumeResultFunc_ = nullptr;
    ret = ssm_->NotifyWatchGestureConsumeResult(keyCode, isConsumed);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: NotifyWatchFocusActiveChange
 * @tc.desc: SceneSesionManager test NotifyWatchFocusActiveChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, NotifyWatchFocusActiveChange, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    bool isActive = true;
    ssm_->onWatchFocusActiveChangeFunc_ = [](bool isActive) {};
    auto ret = ssm_->NotifyWatchFocusActiveChange(isActive);
    ASSERT_EQ(ret, WMError::WM_OK);

    ssm_->onWatchFocusActiveChangeFunc_ = nullptr;
    ret = ssm_->NotifyWatchFocusActiveChange(isActive);
    ASSERT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: DestroyUIServiceExtensionSubWindow
 * @tc.desc: SceneSesionManager test DestroyUIServiceExtensionSubWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, DestroyUIServiceExtensionSubWindow, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->DestroyUIServiceExtensionSubWindow(sceneSession);
    SessionInfo sessionInfo = { "bundleName", "moduleName", "abilityName" };
    sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ssm_->DestroyUIServiceExtensionSubWindow(sceneSession);

    sptr<WindowSessionProperty> property_ = sptr<WindowSessionProperty>::MakeSptr();
    property_->isUIExtFirstSubWindow_ = true;
    ASSERT_EQ(property_->isUIExtAnySubWindow_, false);
    ssm_->DestroyUIServiceExtensionSubWindow(sceneSession);
}

/**
 * @tc.name: FilterForGetAllWindowLayoutInfo
 * @tc.desc: SceneSesionManager test FilterForGetAllWindowLayoutInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, FilterForGetAllWindowLayoutInfo, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    DisplayId displayId = 0;
    bool isVirtualDisplay = true;
    std::vector<sptr<SceneSession>> filteredSessions{};
    SessionInfo info;
    info.bundleName_ = BUNDLE_NAME;
    info.appInstanceKey_ = "instanceKey";
    info.isNewAppInstance_ = true;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    ssm_->sceneSessionMap_.clear();
    auto ret = ssm_->sceneSessionMap_.size();
    ASSERT_EQ(ret, 0);
    ssm_->FilterForGetAllWindowLayoutInfo(displayId, isVirtualDisplay, filteredSessions);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->FilterForGetAllWindowLayoutInfo(displayId, isVirtualDisplay, filteredSessions);
}

/**
 * @tc.name: ShiftAppWindowPointerEvent
 * @tc.desc: SceneSesionManager test ShiftAppWindowPointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, ShiftAppWindowPointerEvent, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    int32_t sourcePersistentId = 0;
    int32_t targetPersistentId = 0;
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    auto ret = ssm_->systemConfig_.IsPcWindow();
    ASSERT_EQ(ret, false);

    int32_t fingerId = 0;
    auto res = ssm_->ShiftAppWindowPointerEvent(sourcePersistentId, targetPersistentId, fingerId);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_SESSION);

    SessionInfo sourceInfo;
    sourceInfo.windowType_ = 1;
    sptr<SceneSession> sourceSceneSession = sptr<SceneSession>::MakeSptr(sourceInfo, nullptr);
    ssm_->sceneSessionMap_.insert({ sourceSceneSession->GetPersistentId(), sourceSceneSession });
    res = ssm_->ShiftAppWindowPointerEvent(sourceSceneSession->GetPersistentId(), targetPersistentId, fingerId);
    EXPECT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ssm_->systemConfig_.freeMultiWindowEnable_ = true;
    ssm_->systemConfig_.freeMultiWindowSupport_ = true;
    res = ssm_->ShiftAppWindowPointerEvent(sourceSceneSession->GetPersistentId(),
        sourceSceneSession->GetPersistentId(), fingerId);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_CALLING);
    ssm_->sceneSessionMap_.erase(sourceSceneSession->GetPersistentId());
}

/**
 * @tc.name: HasFloatingWindowForeground
 * @tc.desc: SceneSesionManager test HasFloatingWindowForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, HasFloatingWindowForeground, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IRemoteObject> abilityToken = nullptr;
    bool hasOrNot = true;
    auto ret = ssm_->HasFloatingWindowForeground(abilityToken, hasOrNot);
    ASSERT_EQ(ret, WMError::WM_ERROR_NULLPTR);
}

/**
 * @tc.name: SetParentWindow
 * @tc.desc: SceneSesionManager test SetParentWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, SetParentWindow, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    ASSERT_NE(ssm_, nullptr);
    int32_t subWindowId = 1;
    int32_t newParentWindowId = 3;
    auto res = ssm_->SetParentWindow(subWindowId, newParentWindowId);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_WINDOW);
    SessionInfo info;
    info.abilityName_ = "SetParentWindow";
    info.bundleName_ = "SetParentWindow";
    sptr<SceneSession> subSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    subSession->property_->SetPersistentId(1);
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ssm_->sceneSessionMap_.insert({ subSession->property_->GetPersistentId(), subSession });
    res = ssm_->SetParentWindow(subWindowId, newParentWindowId);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARENT);

    sptr<SceneSession> oldParentSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    oldParentSession->property_->SetPersistentId(2);
    oldParentSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    ssm_->sceneSessionMap_.insert({ oldParentSession->property_->GetPersistentId(), oldParentSession });
    subSession->property_->SetParentPersistentId(2);
    subSession->SetParentSession(oldParentSession);
    res = ssm_->SetParentWindow(subWindowId, newParentWindowId);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARENT);

    sptr<SceneSession> newParentSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    newParentSession->property_->SetPersistentId(3);
    newParentSession->property_->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ssm_->sceneSessionMap_.insert({ newParentSession->property_->GetPersistentId(), newParentSession });
    res = ssm_->SetParentWindow(subWindowId, newParentWindowId);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARENT);

    newParentSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    res = ssm_->SetParentWindow(subWindowId, newParentWindowId);
    EXPECT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: GetMainSessionByBundleNameAndAppIndex
 * @tc.desc: GetMainSessionByBundleNameAndAppIndex
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetMainSessionByBundleNameAndAppIndex, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    std::string bundleName = "bundleName_test";
    int32_t appIndex = 1;
    std::vector<sptr<SceneSession>> mainSessions;
    ssm_->sceneSessionMap_.clear();
    ssm_->GetMainSessionByBundleNameAndAppIndex(bundleName, appIndex, mainSessions);

    sptr<SceneSession> sceneSession = GetSceneSession(bundleName);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->GetMainSessionByBundleNameAndAppIndex(bundleName, appIndex, mainSessions);

    ssm_->GetMainSessionByBundleNameAndAppIndex(BUNDLE_NAME, appIndex, mainSessions);

    appIndex = 0;
    ssm_->GetMainSessionByBundleNameAndAppIndex(BUNDLE_NAME, appIndex, mainSessions);
    ASSERT_EQ(mainSessions.empty(), false);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: GetMainSessionByAbilityInfo
 * @tc.desc: GetMainSessionByAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetMainSessionByAbilityInfo, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    AbilityInfoBase abilityInfo;
    abilityInfo.bundleName = "bundleName";
    abilityInfo.moduleName = "moduleName";
    abilityInfo.abilityName = "abilityName";
    abilityInfo.appIndex = 1;
    std::vector<sptr<SceneSession>> mainSessions;
    std::string bundleName = "bundleName_test";
    sptr<SceneSession> sceneSession = GetSceneSession(bundleName);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->GetMainSessionByAbilityInfo(abilityInfo, mainSessions);

    abilityInfo.bundleName = BUNDLE_NAME;
    ssm_->GetMainSessionByAbilityInfo(abilityInfo, mainSessions);

    abilityInfo.moduleName = "";
    ssm_->GetMainSessionByAbilityInfo(abilityInfo, mainSessions);

    abilityInfo.abilityName = "";
    ssm_->GetMainSessionByAbilityInfo(abilityInfo, mainSessions);

    abilityInfo.appIndex = 0;
    ssm_->GetMainSessionByAbilityInfo(abilityInfo, mainSessions);
    ASSERT_EQ(mainSessions.empty(), false);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: GetKeyboardSession
 * @tc.desc: GetKeyboardSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetKeyboardSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    DisplayId displayId = DISPLAY_ID_INVALID;
    bool isSystemKeyboard = true;
    ASSERT_EQ(ssm_->GetKeyboardSession(displayId, isSystemKeyboard), nullptr);

    displayId = 0;
    ssm_->GetKeyboardSession(displayId, isSystemKeyboard);

    SessionInfo info;
    info.bundleName_ = BUNDLE_NAME;
    info.screenId_ = 5;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->GetKeyboardSession(displayId, isSystemKeyboard);

    displayId = 5;
    ssm_->GetKeyboardSession(displayId, isSystemKeyboard);

    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    ssm_->GetKeyboardSession(displayId, isSystemKeyboard);

    isSystemKeyboard = false;
    sptr<SceneSession> keyboardSession = ssm_->GetKeyboardSession(displayId, isSystemKeyboard);
    ASSERT_EQ(keyboardSession, sceneSession);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: DestroyToastSession
 * @tc.desc: DestroyToastSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, DestroyToastSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    std::string bundleName = "bundleName_test";
    sptr<SceneSession> sceneSession = GetSceneSession(bundleName);
    ssm_->DestroyToastSession(nullptr);
    ssm_->DestroyToastSession(sceneSession);

    sptr<SceneSession> sceneSession02 = GetSceneSession(bundleName);
    sceneSession->toastSession_.emplace_back(sceneSession02);
    ssm_->DestroyToastSession(sceneSession);
    ASSERT_EQ(sceneSession->toastSession_.empty(), false);
}

/**
 * @tc.name: CreateAndConnectSpecificSession01
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, CreateAndConnectSpecificSession01, TestSize.Level0)
{
    sptr<ISessionStage> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<IWindowEventChannel> eventChannel = sptr<WindowEventChannelMocker>::MakeSptr(sessionStage);
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    int32_t persistentId = 1;
    sptr<ISession> session = nullptr;
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();

    property->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    auto result = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, persistentId, session, systemConfig, iRemoteObjectMocker);
    ASSERT_EQ(result, WSError::WS_ERROR_NOT_SYSTEM_APP);

    property->SetTopmost(false);
    property->SetWindowType(WindowType::WINDOW_TYPE_MEDIA);
    std::string bundleName = "bundleName_test";
    sptr<SceneSession> parentSession = GetSceneSession(bundleName);
    parentSession->GetSessionProperty()->SetSubWindowLevel(10);
    ssm_->sceneSessionMap_.insert({ 1, parentSession });
    property->SetParentPersistentId(1);
    result = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, persistentId, session, systemConfig, iRemoteObjectMocker);
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_WINDOW);

    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    parentSession->GetSessionProperty()->SetSubWindowLevel(1);
    property->SetWindowType(WindowType::WINDOW_TYPE_FB);
    result = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, persistentId, session, systemConfig, iRemoteObjectMocker);
    ASSERT_EQ(WSError::WS_ERROR_NOT_SYSTEM_APP, result);
    MockAccesstokenKit::MockAccessTokenKitRet(0);
    parentSession->SetSessionState(SessionState::STATE_DISCONNECT);
    result = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, persistentId, session, systemConfig, iRemoteObjectMocker);
    ASSERT_EQ(WSError::WS_ERROR_INVALID_PARENT, result);
    parentSession->SetSessionState(SessionState::STATE_FOREGROUND);
    result = ssm_->CreateAndConnectSpecificSession(
        sessionStage, eventChannel, node, property, persistentId, session, systemConfig, iRemoteObjectMocker);
    ASSERT_EQ(WSError::WS_OK, result);
}

/**
 * @tc.name: IsLastPiPWindowVisible
 * @tc.desc: IsLastPiPWindowVisible
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, IsLastPiPWindowVisible, TestSize.Level1)
{
    uint64_t surfaceId = 1;
    WindowVisibilityState lastVisibilityState = WindowVisibilityState::WINDOW_VISIBILITY_STATE_NO_OCCLUSION;
    ssm_->sceneSessionMap_.clear();
    auto res = ssm_->IsLastPiPWindowVisible(surfaceId, lastVisibilityState);
    ASSERT_EQ(res, false);
}

/**
 * @tc.name: GetIconFromDesk
 * @tc.desc: GetIconFromDesk
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetIconFromDesk, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = BUNDLE_NAME;
    sessionInfo.bundleName_ = BUNDLE_NAME;
    std::string startupPagePath = "test";
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    sessionInfo.want = want;
    ASSERT_EQ(false, ssm_->GetIconFromDesk(sessionInfo, startupPagePath));
}

/**
 * @tc.name: GetTopNearestBlockingFocusSession
 * @tc.desc: GetTopNearestBlockingFocusSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetTopNearestBlockingFocusSession, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();
    DisplayId displayId = DEFAULT_DISPLAY_ID;
    uint32_t zOrder = 0;
    bool includingAppSession = true;
    ASSERT_EQ(nullptr, ssm_->GetTopNearestBlockingFocusSession(displayId, zOrder, includingAppSession));

    SessionInfo info;
    info.bundleName_ = BUNDLE_NAME;
    info.isSystem_ = true;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info, specificCb);
    sceneSession->GetSessionProperty()->SetDisplayId(DEFAULT_DISPLAY_ID);
    sceneSession->zOrder_ = 10;
    sceneSession->GetSessionProperty()->SetTopmost(true);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::SYSTEM_WINDOW_BASE);
    sceneSession->GetSessionProperty()->SetParentPersistentId(2);
    sceneSession->SetScbCoreEnabled(true);
    sceneSession->isVisible_ = true;
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->blockingFocus_ = true;
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->sceneSessionMap_.insert({ 2, sceneSession02 });
    auto res = ssm_->GetTopNearestBlockingFocusSession(displayId, zOrder, includingAppSession);
    ASSERT_EQ(res, sceneSession);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: GetTopNearestBlockingFocusSession_branch02
 * @tc.desc: GetTopNearestBlockingFocusSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetTopNearestBlockingFocusSession_branch02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();
    DisplayId displayId = DEFAULT_DISPLAY_ID;
    uint32_t zOrder = 0;
    bool includingAppSession = true;
    ASSERT_EQ(nullptr, ssm_->GetTopNearestBlockingFocusSession(displayId, zOrder, includingAppSession));

    SessionInfo info;
    info.bundleName_ = BUNDLE_NAME;
    info.isSystem_ = true;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sceneSession->GetSessionProperty()->SetDisplayId(100);
    ssm_->windowFocusController_->displayId2GroupIdMap_[100] = 20;
    ssm_->windowFocusController_->displayId2GroupIdMap_[20] = 20;
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ASSERT_EQ(nullptr, ssm_->GetTopNearestBlockingFocusSession(displayId, zOrder, includingAppSession));
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: GetTopNearestBlockingFocusSession_branch03
 * @tc.desc: GetTopNearestBlockingFocusSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetTopNearestBlockingFocusSession_branch03, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();
    DisplayId displayId = DEFAULT_DISPLAY_ID;
    uint32_t zOrder = 100;
    bool includingAppSession = true;
    ASSERT_EQ(nullptr, ssm_->GetTopNearestBlockingFocusSession(displayId, zOrder, includingAppSession));

    SessionInfo info;
    info.bundleName_ = BUNDLE_NAME;
    info.isSystem_ = true;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sceneSession->GetSessionProperty()->SetDisplayId(DEFAULT_DISPLAY_ID);
    sceneSession->zOrder_ = 10;
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ASSERT_EQ(nullptr, ssm_->GetTopNearestBlockingFocusSession(displayId, zOrder, includingAppSession));
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: GetTopNearestBlockingFocusSession_branch04
 * @tc.desc: GetTopNearestBlockingFocusSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetTopNearestBlockingFocusSession_branch04, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();
    DisplayId displayId = DEFAULT_DISPLAY_ID;
    uint32_t zOrder = 0;
    bool includingAppSession = true;
    ASSERT_EQ(nullptr, ssm_->GetTopNearestBlockingFocusSession(displayId, zOrder, includingAppSession));

    SessionInfo info;
    info.bundleName_ = BUNDLE_NAME;
    info.isSystem_ = true;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sceneSession->GetSessionProperty()->SetDisplayId(DEFAULT_DISPLAY_ID);
    sceneSession->GetSessionProperty()->SetTopmost(true);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->zOrder_ = 10;
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ASSERT_EQ(nullptr, ssm_->GetTopNearestBlockingFocusSession(displayId, zOrder, includingAppSession));
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: GetTopNearestBlockingFocusSession_branch05
 * @tc.desc: GetTopNearestBlockingFocusSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetTopNearestBlockingFocusSession_branch05, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();
    DisplayId displayId = DEFAULT_DISPLAY_ID;
    uint32_t zOrder = 0;
    bool includingAppSession = true;
    ASSERT_EQ(nullptr, ssm_->GetTopNearestBlockingFocusSession(displayId, zOrder, includingAppSession));

    SessionInfo info;
    info.bundleName_ = BUNDLE_NAME;
    info.isSystem_ = true;
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sptr<SceneSession> sceneSession02 = sptr<SceneSession>::MakeSptr(info, specificCb);
    sceneSession->GetSessionProperty()->SetDisplayId(DEFAULT_DISPLAY_ID);
    sceneSession->GetSessionProperty()->SetTopmost(false);
    sceneSession->zOrder_ = 10;
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::APP_WINDOW_BASE);
    sceneSession->GetSessionProperty()->SetParentPersistentId(2);
    sceneSession02->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession02->GetSessionProperty()->SetTopmost(true);
    ssm_->sceneSessionMap_.insert({ 1, sceneSession });
    ssm_->sceneSessionMap_.insert({ 2, sceneSession02 });

    ASSERT_EQ(nullptr, ssm_->GetTopNearestBlockingFocusSession(displayId, zOrder, includingAppSession));
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: AnimateTo01
 * @tc.desc: AnimateTo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, AnimateTo01, Function | SmallTest | Level1)
{
    SessionInfo info;
    info.bundleName_ = "AnimateToTest";
    sptr<SceneSession::SpecificSessionCallback> specificCb = sptr<SceneSession::SpecificSessionCallback>::MakeSptr();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, specificCb);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::APP_WINDOW_BASE);
    float targetScale = 0;
    WindowAnimationCurve curve = WindowAnimationCurve::LINEAR;
    sceneSession->RegisterAnimateToCallback([&targetScale, &curve](const WindowAnimationProperty& animationProperty,
        const WindowAnimationOption& animationOption) {
        targetScale = animationProperty.targetScale;
        curve = animationOption.curve;
    });

    auto persistenId = sceneSession->GetPersistentId();
    ssm_->sceneSessionMap_.insert({ persistenId, sceneSession });
    WindowAnimationProperty animationProperty;
    animationProperty.targetScale = 10.5f;
    WindowAnimationOption animationOption;
    animationOption.curve = WindowAnimationCurve::INTERPOLATION_SPRING;
    animationOption.duration = 1000;

    ssm_->AnimateTo(0, animationProperty, animationOption);
    usleep(SLEEP_TIME);
    ASSERT_EQ(curve, WindowAnimationCurve::LINEAR);
    ASSERT_EQ(targetScale, 0);

    sceneSession->GetSessionProperty()->SetWindowType(WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE);
    ssm_->AnimateTo(persistenId, animationProperty, animationOption);
    usleep(SLEEP_TIME);
    ASSERT_EQ(curve, WindowAnimationCurve::LINEAR);
    ASSERT_EQ(targetScale, 0);

    sceneSession->GetSessionProperty()->SetWindowType(WindowType::APP_WINDOW_BASE);
    ssm_->AnimateTo(persistenId, animationProperty, animationOption);
    usleep(SLEEP_TIME);
    ASSERT_EQ(curve, WindowAnimationCurve::INTERPOLATION_SPRING);
    ASSERT_EQ(targetScale, animationProperty.targetScale);
}

/**
 * @tc.name: UpdateHighlightStatus
 * @tc.desc: UpdateHighlightStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, UpdateHighlightStatus, TestSize.Level1)
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
    EXPECT_EQ(ssm_->highlightIds_.size(), 0);

    ssm_->AddHighlightSessionIds(preSceneSession, false);
    EXPECT_EQ(ssm_->highlightIds_.size(), 1);
    ssm_->UpdateHighlightStatus(DEFAULT_DISPLAY_ID, preSceneSession, nullSceneSession2, false);
    EXPECT_EQ(ssm_->highlightIds_.size(), 1);
    ssm_->UpdateHighlightStatus(DEFAULT_DISPLAY_ID, preSceneSession, nullSceneSession2, true);
    EXPECT_EQ(ssm_->highlightIds_.size(), 0);

    ssm_->UpdateHighlightStatus(DEFAULT_DISPLAY_ID, preSceneSession, currSceneSession, true);
    ssm_->UpdateHighlightStatus(DEFAULT_DISPLAY_ID, preSceneSession, currSceneSession, false);
    EXPECT_EQ(ssm_->highlightIds_.size(), 1);

    currSceneSession->property_->isExclusivelyHighlighted_ = false;
    preSceneSession->property_->SetPersistentId(2);
    ssm_->UpdateHighlightStatus(DEFAULT_DISPLAY_ID, preSceneSession, currSceneSession, false);
}

/**
 * @tc.name: UpdateHighlightStatus01
 * @tc.desc: UpdateHighlightStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, UpdateHighlightStatus01, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest11, SetHighlightSessionIds, TestSize.Level1)
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
    auto timeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    ssm_->SetHighlightSessionIds(currSceneSession, false, timeStamp);
    EXPECT_EQ(ssm_->highlightIds_.count(1) == 1, true);
    auto samePidSession = sptr<SceneSession>::MakeSptr(info1, nullptr);
    samePidSession->SetCallingPid(currSceneSession->GetCallingPid());
    samePidSession->persistentId_ = 321;
    ssm_->AddHighlightSessionIds(samePidSession, false);
    ssm_->SetHighlightSessionIds(currSceneSession, false, timeStamp);
    EXPECT_EQ(ssm_->highlightIds_.count(1) == 1, true);
}

/**
 * @tc.name: AddHighlightSessionIds
 * @tc.desc: AddHighlightSessionIds
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, AddHighlightSessionIds, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest11, RemoveHighlightSessionIds, TestSize.Level1)
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
 * @tc.name: RemoveLifeCycleTaskByPersistentId
 * @tc.desc: test RemoveLifeCycleTaskByPersistentId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, RemoveLifeCycleTaskByPersistentId, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest11, SetStatusBarAvoidHeight, TestSize.Level1)
{
    int32_t height = 10;
    int32_t height2 = -1;
    ssm_->SetStatusBarAvoidHeight(0, height);
    ssm_->SetStatusBarAvoidHeight(1, height2);
    WSRect barArea = { 0, 0, 100, 100 };
    WSRect barArea2 = { 0, 0, 100, 100 };
    WSRect barArea3 = { 0, 0, 100, 100 };
    ssm_->GetStatusBarAvoidHeight(0, barArea);
    EXPECT_EQ(barArea.height_, height);
    ssm_->GetStatusBarAvoidHeight(1, barArea2);
    EXPECT_EQ(barArea2.height_, 100);
    ssm_->GetStatusBarAvoidHeight(2, barArea3);
    EXPECT_EQ(barArea3.height_, 100);
}

/**
 * @tc.name: QueryAbilityInfoFromBMSTest
 * @tc.desc: SceneSesionManager QueryAbilityInfoFromBMS NotifyStartAbility
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, QueryAbilityInfoFromBMSTest, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest11, QueryAbilityInfoFromBMSTest001, TestSize.Level1)
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
 * @tc.name: QueryAbilityInfoFromBMSTest
 * @tc.desc: SceneSesionManager QueryAbilityInfoFromBMS AtomicFreeInstall query failed
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, QueryAbilityInfoFromBMSTest02, TestSize.Level1)
{
    const int32_t uId = 32;
    SessionInfo sessionInfo_;
    sessionInfo_.bundleName_ = "BundleName";
    sessionInfo_.abilityName_ = "AbilityName";
    sessionInfo_.moduleName_ = "ModuleName";
    sptr<IBundleMgrMocker> bundleMgrMocker = sptr<IBundleMgrMocker>::MakeSptr();
    EXPECT_CALL(*bundleMgrMocker, GetBundleInfoV9(_, _, _, _)).WillOnce(Return(1));
    ssm_->bundleMgr_ = bundleMgrMocker;

    auto res = ssm_->QueryAbilityInfoFromBMS(
        uId, sessionInfo_.bundleName_, sessionInfo_.abilityName_, sessionInfo_.moduleName_, true);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: QueryAbilityInfoFromBMSTest
 * @tc.desc: SceneSesionManager QueryAbilityInfoFromBMS AtomicFreeInstall query failed hapModuleInfosis nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, QueryAbilityInfoFromBMSTest03, TestSize.Level1)
{
    const int32_t uId = 32;
    SessionInfo sessionInfo_;
    sessionInfo_.bundleName_ = "BundleName";
    sessionInfo_.abilityName_ = "";
    sessionInfo_.moduleName_ = "";
    sptr<IBundleMgrMocker> bundleMgrMocker = sptr<IBundleMgrMocker>::MakeSptr();
    ssm_->bundleMgr_ = bundleMgrMocker;

    auto res = ssm_->QueryAbilityInfoFromBMS(
        uId, sessionInfo_.bundleName_, sessionInfo_.abilityName_, sessionInfo_.moduleName_, true);
    EXPECT_EQ(res, nullptr);
}

/**
 * @tc.name: QueryAbilityInfoFromBMSTest
 * @tc.desc: SceneSesionManager QueryAbilityInfoFromBMS AtomicFreeInstall query success.
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, QueryAbilityInfoFromBMSTest04, TestSize.Level1)
{
    const int32_t uId = 32;
    SessionInfo sessionInfo_;
    sessionInfo_.bundleName_ = "BundleName";
    sessionInfo_.abilityName_ = "";
    sessionInfo_.moduleName_ = "";
    sptr<IBundleMgrMocker> bundleMgrMocker = sptr<IBundleMgrMocker>::MakeSptr();
    EXPECT_CALL(*bundleMgrMocker, GetBundleInfoV9(_, _, _, _))
        .WillOnce([](const std::string& bundleName, int32_t flags, AppExecFwk::BundleInfo& bundleInfo, int32_t userId) {
            AppExecFwk::AbilityInfo abilityInfo;
            abilityInfo.moduleName = "moduleName";
            abilityInfo.name = "abilityName";
            AppExecFwk::HapModuleInfo hapModuleInfo;
            hapModuleInfo.abilityInfos = { abilityInfo };
            bundleInfo.hapModuleInfos = { hapModuleInfo };
            return 0;
        });
    ssm_->bundleMgr_ = bundleMgrMocker;

    auto res = ssm_->QueryAbilityInfoFromBMS(
        uId, sessionInfo_.bundleName_, sessionInfo_.abilityName_, sessionInfo_.moduleName_, true);
    ASSERT_EQ(res, nullptr);
    EXPECT_EQ(res->name, "abilityName");
    EXPECT_EQ(res->moduleName, "moduleName");
}

/**
 * @tc.name: RequestFocusSpecificCheckTest
 * @tc.desc: Test for RequestFocusSpecificCheck
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, RequestFocusSpecificCheckTest, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest11, NotifyUnFocusedByMissionTest001, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest11, NotifyUnFocusedByMissionTest002, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest11, NotifyUnFocusedByMissionTest003, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest11, NotifyStackEmptyTest, TestSize.Level1)
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
HWTEST_F(SceneSessionManagerTest11, AddSkipSelfWhenShowOnVirtualScreenList, Function | SmallTest | Level1)
{
    SessionInfo info;
    info.bundleName_ = "AddSkipSelfWhenShowOnVirtualScreenList";
    info.abilityName_ = "AddSkipSelfWhenShowOnVirtualScreenList";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    std::vector<int32_t> persistentIds{ sceneSession->GetPersistentId() };
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(false);
    auto ret = ssm_->AddSkipSelfWhenShowOnVirtualScreenList(persistentIds);
    EXPECT_EQ(ret, WMError::WM_ERROR_NOT_SYSTEM_APP);

    MockAccesstokenKit::MockIsSACalling(true);
    MockAccesstokenKit::MockIsSystemApp(true);
    ssm_->isUserBackground_ = false;
    ret = ssm_->AddSkipSelfWhenShowOnVirtualScreenList(persistentIds);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(ret, WMError::WM_OK);

    ssm_->isUserBackground_ = true;
    ret = ssm_->AddSkipSelfWhenShowOnVirtualScreenList(persistentIds);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: RemoveSkipSelfWhenShowOnVirtualScreenList
 * @tc.desc: test function : RemoveSkipSelfWhenShowOnVirtualScreenList
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, RemoveSkipSelfWhenShowOnVirtualScreenList, Function | SmallTest | Level1)
{
    SessionInfo info;
    info.bundleName_ = "RemoveSkipSelfWhenShowOnVirtualScreenList";
    info.abilityName_ = "RemoveSkipSelfWhenShowOnVirtualScreenList";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->sceneSessionMap_.insert({ sceneSession->GetPersistentId(), sceneSession });
    std::vector<int32_t> persistentIds{ sceneSession->GetPersistentId() };
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(false);
    auto ret = ssm_->RemoveSkipSelfWhenShowOnVirtualScreenList(persistentIds);
    EXPECT_EQ(ret, WMError::WM_ERROR_NOT_SYSTEM_APP);

    MockAccesstokenKit::MockIsSACalling(true);
    MockAccesstokenKit::MockIsSystemApp(true);
    ssm_->isUserBackground_ = false;
    ret = ssm_->RemoveSkipSelfWhenShowOnVirtualScreenList(persistentIds);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(ret, WMError::WM_OK);

    ssm_->isUserBackground_ = true;
    ret = ssm_->RemoveSkipSelfWhenShowOnVirtualScreenList(persistentIds);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(ret, WMError::WM_OK);
}

/**
 * @tc.name: GetHookedSessionByModuleName
 * @tc.desc: test function : GetHookedSessionByModuleName
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetHookedSessionByModuleName, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerTest11, RequestSceneSession, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerTest11, UpdateAbilityHookState, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerTest11, UpdateRecentMainSessionInfos, Function | SmallTest | Level2)
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
HWTEST_F(SceneSessionManagerTest11, GetRecentMainSessionInfoList, Function | SmallTest | Level2)
{
    std::vector<RecentSessionInfo> recentSessionInfoList = {};
    auto result = ssm_->GetRecentMainSessionInfoList(recentSessionInfoList);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_PERMISSION);
}

/**
 * @tc.name: GetVisibilityWindowInfo
 * @tc.desc: test whether get the visibility window information
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetVisibilityWindowInfo, Function | SmallTest | Level2)
{
    auto oldVisibleData = ssm_->lastVisibleData_;
    auto oldSessionMap = ssm_->sceneSessionMap_;
    SessionInfo sessionInfo;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    struct RSSurfaceNodeConfig surfaceNodeConfig;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    ASSERT_NE(surfaceNode, nullptr);
    sceneSession->SetSurfaceNode(surfaceNode);
    std::map<int32_t, sptr<SceneSession>> currSessionMap;
    currSessionMap.insert({ sceneSession->GetPersistentId(), sceneSession });
    ssm_->sceneSessionMap_ = currSessionMap;
    std::vector<std::pair<uint64_t, WindowVisibilityState>> currVisibleData;
    currVisibleData.emplace_back(surfaceNode->GetId(), WindowVisibilityState::START);
    ssm_->lastVisibleData_ = currVisibleData;
    std::vector<sptr<WindowVisibilityInfo>> infos;
    auto result = ssm_->GetVisibilityWindowInfo(infos);
    EXPECT_EQ(result, WMError::WM_OK);
    EXPECT_EQ(infos.size(), 1);
    ssm_->lastVisibleData_ = oldVisibleData;
    ssm_->sceneSessionMap_ = oldSessionMap;
}

/**
 * @tc.name: SendPointerEventForHover
 * @tc.desc: SendPointerEventForHover
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, SendPointerEventForHover_Vaild, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    MockAccesstokenKit::MockIsSACalling(false);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = nullptr;
    WSError ret = ssm_->SendPointerEventForHover(pointerEvent);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_PERMISSION);

    MockAccesstokenKit::MockIsSACalling(true);
    ret = ssm_->SendPointerEventForHover(pointerEvent);
    EXPECT_EQ(ret, WSError::WS_ERROR_NULLPTR);

    pointerEvent = MMI::PointerEvent::Create();
    ret = ssm_->SendPointerEventForHover(pointerEvent);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_PARAM);

    pointerEvent->pointerAction_ = MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER;
    pointerEvent->sourceType_ = MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    pointerEvent->agentWindowId_ = 1;
    ret = ssm_->SendPointerEventForHover(pointerEvent);
    EXPECT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
}

/**
 * @tc.name: SendPointerEventForHover_Success
 * @tc.desc: SendPointerEventForHover
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, SendPointerEventForHover_Success, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    MockAccesstokenKit::MockIsSACalling(true);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    pointerEvent->pointerAction_ = MMI::PointerEvent::POINTER_ACTION_HOVER_ENTER;
    pointerEvent->sourceType_ = MMI::PointerEvent::SOURCE_TYPE_TOUCHSCREEN;
    pointerEvent->agentWindowId_ = 1;

    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerTest11";
    sessionInfo.abilityName_ = "SendPointerEventForHover";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->persistentId_ = 1;
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    WSError ret = ssm_->SendPointerEventForHover(pointerEvent);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: TestCheckSystemWindowPermission_Fb
 * @tc.desc: Test CheckSystemWindowPermission with windowType WINDOW_TYPE_FB then true
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, TestCheckSystemWindowPermission_Fb, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();

    property->SetWindowType(WindowType::WINDOW_TYPE_FB);
    ASSERT_EQ(true, ssm_->CheckSystemWindowPermission(property));
}

/**
 * @tc.name: InitFbWindow
 * @tc.desc: test function : InitFbWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, InitFbWindow, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    SessionInfo sessionInfo;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);

    ssm_->InitFbWindow(sceneSession, nullptr);

    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_PIP);
    ssm_->InitFbWindow(sceneSession, property);

    property->SetWindowType(WindowType::WINDOW_TYPE_FB);
    ssm_->InitFbWindow(sceneSession, property);
    EXPECT_EQ(0, sceneSession->GetFbTemplateInfo().template_);
}

/**
 * @tc.name: GetFbPanelWindowId
 * @tc.desc: test function : GetFbPanelWindowId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetFbPanelWindowId, TestSize.Level1)
{
    ASSERT_NE(nullptr, ssm_);
    uint32_t windowId = 0;
    EXPECT_EQ(WMError::WM_ERROR_FB_INTERNAL_ERROR, ssm_->GetFbPanelWindowId(windowId));
    ssm_->sceneSessionMap_.insert({0, nullptr});
    ssm_->sceneSessionMap_.insert({1, CreateSceneSession("", WindowType::WINDOW_TYPE_PIP)});
    ssm_->sceneSessionMap_.insert({2, CreateSceneSession("SCBGlobalSearch7", WindowType::WINDOW_TYPE_FB)});
    sptr<SceneSession> sceneSession = CreateSceneSession("Fb_panel8", WindowType::WINDOW_TYPE_FB);
    ssm_->sceneSessionMap_.insert({3, sceneSession});

    MockAccesstokenKit::MockAccessTokenKitRet(0);
    EXPECT_EQ(WMError::WM_OK, ssm_->GetFbPanelWindowId(windowId));
    EXPECT_EQ(sceneSession->GetWindowId(), windowId);
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: ConfigSupportCreateFloatWindow
 * @tc.desc: test function : ConfigSupportCreateFloatWindow
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, ConfigSupportCreateFloatWindow, TestSize.Level0)
{
    ASSERT_NE(nullptr, ssm_);

    ssm_->ConfigSupportCreateFloatWindow();

    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(ssm_->systemConfig_.supportCreateFloatWindow_);
}

/**
 * @tc.name: ConfigDockAutoHide
 * @tc.desc: test function : ConfigDockAutoHide
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, ConfigDockAutoHide, TestSize.Level0)
{
    ssm_->ConfigDockAutoHide(true);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(ssm_->systemConfig_.isDockAutoHide_);

    ssm_->ConfigDockAutoHide(false);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_FALSE(ssm_->systemConfig_.isDockAutoHide_);
}

/**
 * @tc.name: SetSpecificWindowZIndexListener
 * @tc.desc: test function : SetSpecificWindowZIndexListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, SetSpecificWindowZIndexListener, TestSize.Level1)
{
    int value = 0;
    NotifySetSpecificWindowZIndexFunc func = [&value](WindowType windowType, int32_t zIndex,
        SetSpecificZIndexReason reason) {
        value = zIndex;
    };
    ssm_->SetSpecificWindowZIndexListener(func);
    ssm_->setSpecificWindowZIndexFunc_(WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD, 20, SetSpecificZIndexReason::SET);
    EXPECT_EQ(value, 20);
    ssm_->SetSpecificWindowZIndexListener(nullptr);
}

/**
 * @tc.name: SetSpecificWindowZIndex
 * @tc.desc: test function : SetSpecificWindowZIndex
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, SetSpecificWindowZIndex, TestSize.Level1)
{
    MockAccesstokenKit::MockIsSystemApp(false);
    WSError ret = ssm_->SetSpecificWindowZIndex(WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD, 20);
    EXPECT_EQ(ret, WSError::WS_ERROR_NOT_SYSTEM_APP);

    MockAccesstokenKit::MockIsSystemApp(true);
    int value = 0;
    NotifySetSpecificWindowZIndexFunc func = [&value](WindowType windowType, int32_t zIndex,
        SetSpecificZIndexReason reason) {
        value = zIndex;
    };
    ssm_->SetSpecificWindowZIndexListener(func);
    ret = ssm_->SetSpecificWindowZIndex(WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD, 20);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_EQ(value, 20);
    ssm_->SetSpecificWindowZIndexListener(nullptr);
}

/**
 * @tc.name: SetBufferAvailable
 * @tc.desc: test function : SetBufferAvailable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, SetBufferAvailable1, Function | SmallTest | Level2)
{
    sptr<SceneSession> sceneSession = nullptr;
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ssm_->SetBufferAvailable(sceneSession);
    EXPECT_TRUE(g_logMsg.find("scene session is nullptr") != std::string::npos) << "Check sceneSession is null.";

    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetBufferAvailable
 * @tc.desc: test function : SetBufferAvailable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, SetBufferAvailable2, Function | SmallTest | Level2)
{
    SessionInfo sessionInfo;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::BELOW_APP_SYSTEM_WINDOW_END);

    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ssm_->SetBufferAvailable(sceneSession);
    EXPECT_TRUE(
        g_logMsg.find("enter buffer available callback") == std::string::npos) << "Check WindowType is invalid.";

    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetBufferAvailable
 * @tc.desc: test function : SetBufferAvailable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, SetBufferAvailable3, Function | SmallTest | Level2)
{
    SessionInfo sessionInfo;
    std::shared_ptr<RSSurfaceNode> surfaceNode = nullptr;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->surfaceNode_ = surfaceNode;
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_DESKTOP);

    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ssm_->SetBufferAvailable(sceneSession);
    EXPECT_TRUE(g_logMsg.find("surface node is nullptr") != std::string::npos) << "Check surfaceNode is null.";

    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetBufferAvailable
 * @tc.desc: test function : SetBufferAvailable
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, SetBufferAvailable4, Function | SmallTest | Level2)
{
    SessionInfo sessionInfo;
    struct RSSurfaceNodeConfig surfaceNodeConfig;

    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(surfaceNodeConfig, RSSurfaceNodeType::DEFAULT);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->surfaceNode_ = surfaceNode;
    sceneSession->GetSessionProperty()->SetWindowType(WindowType::WINDOW_TYPE_KEYGUARD);

    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ssm_->SetBufferAvailable(sceneSession);
    EXPECT_TRUE(g_logMsg.find("config surface node") != std::string::npos);

    g_logMsg.clear();
    LOG_SetCallback(nullptr);
}
} // namespace
} // namespace Rosen
} // namespace OHOS