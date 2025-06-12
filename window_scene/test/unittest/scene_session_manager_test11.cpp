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
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "session_manager.h"
#include "session/host/include/scene_session.h"
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

    static sptr<SceneSessionManager> ssm_;

private:
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
    EXPECT_CALL(*bundleMgrMocker, GetApplicationInfos(_, _, _))
        .WillOnce([modeType, maxCount](const AppExecFwk::ApplicationFlag flag,
                                       const int32_t userId,
                                       std::vector<AppExecFwk::ApplicationInfo>& appInfos) {
            AppExecFwk::ApplicationInfo appInfo;
            appInfo.bundleName = BUNDLE_NAME;
            appInfo.multiAppMode.multiAppModeType = modeType;
            appInfo.multiAppMode.maxCount = maxCount;
            appInfos.push_back(appInfo);
            return true;
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

namespace {
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
    ASSERT_EQ(res, WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ssm_->systemConfig_.freeMultiWindowEnable_ = true;
    ssm_->systemConfig_.freeMultiWindowSupport_ = true;
    res = ssm_->ShiftAppWindowPointerEvent(sourcePersistentId, targetPersistentId, fingerId);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_CALLING);

    sourcePersistentId = 1;
    ssm_->sceneSessionMap_.clear();
    res = ssm_->ShiftAppWindowPointerEvent(sourcePersistentId, targetPersistentId, fingerId);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_SESSION);
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
 * @tc.name: GetStartingWindowInfoFromCache
 * @tc.desc: GetStartingWindowInfoFromCache
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetStartingWindowInfoFromCache, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.moduleName_ = "test";
    sessionInfo.abilityName_ = BUNDLE_NAME;
    sessionInfo.bundleName_ = BUNDLE_NAME;
    StartingWindowInfo startingWindowInfo;
    bool res = ssm_->GetStartingWindowInfoFromCache(sessionInfo, startingWindowInfo);
    ASSERT_EQ(res, false);

    std::map<std::string, StartingWindowInfo> startingWindowInfoMap{ { BUNDLE_NAME, startingWindowInfo } };
    ssm_->startingWindowMap_.insert({ BUNDLE_NAME, startingWindowInfoMap });
    res = ssm_->GetStartingWindowInfoFromCache(sessionInfo, startingWindowInfo);
    ASSERT_EQ(res, false);
    ssm_->startingWindowMap_.clear();
}

/**
 * @tc.name: GetStartingWindowInfoFromCache02
 * @tc.desc: GetStartingWindowInfoFromCache
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetStartingWindowInfoFromCache02, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo sessionInfo;
    sessionInfo.moduleName_ = "te";
    sessionInfo.abilityName_ = "st";
    sessionInfo.bundleName_ = "test";
    StartingWindowInfo startingWindowInfo;

    bool res = ssm_->GetStartingWindowInfoFromCache(sessionInfo, startingWindowInfo);
    ASSERT_EQ(res, false);

    std::map<std::string, StartingWindowInfo> startingWindowInfoMap{ { "test", startingWindowInfo } };
    ssm_->startingWindowMap_.insert({ "test", startingWindowInfoMap });

    res = ssm_->GetStartingWindowInfoFromCache(sessionInfo, startingWindowInfo);
    ASSERT_EQ(res, true);
    ssm_->startingWindowMap_.clear();
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
    ssm_->windowFocusController_->virtualScreenDisplayIdSet_.insert(20);
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
 * @tc.name: GetHostWindowCompatiblityInfo
 * @tc.desc: GetHostWindowCompatiblityInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest11, GetHostWindowCompatiblityInfo, TestSize.Level1)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->sceneSessionMap_.clear();
    sptr<CompatibleModeProperty> property = nullptr;
    sptr<IRemoteObject> token = nullptr;
    WMError res = ssm_->GetHostWindowCompatiblityInfo(token, property);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, res);
    property = sptr<CompatibleModeProperty>::MakeSptr();
    res = ssm_->GetHostWindowCompatiblityInfo(token, property);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, res);
    token = sptr<IRemoteObjectMocker>::MakeSptr();
    res = ssm_->GetHostWindowCompatiblityInfo(token, property);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_WINDOW, res);
    ExtensionWindowAbilityInfo extensionWindowAbilityInfo;
    extensionWindowAbilityInfo.persistentId = 12345958;
    extensionWindowAbilityInfo.parentId = 959;
    ssm_->extSessionInfoMap_.emplace(token, extensionWindowAbilityInfo);
    res = ssm_->GetHostWindowCompatiblityInfo(token, property);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARENT, res);
    
    SessionInfo info;
    info.abilityName_ = "GetHostWindowCompatiblityInfo";
    info.bundleName_ = "GetHostWindowCompatiblityInfo";
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->property_->SetPersistentId(959);
    ssm_->sceneSessionMap_.insert({ session->property_->GetPersistentId(), session });
    res = ssm_->GetHostWindowCompatiblityInfo(token, property);
    EXPECT_EQ(WMError::WM_DO_NOTHING, res);
    sptr<CompatibleModeProperty> compatibleModeProperty = sptr<CompatibleModeProperty>::MakeSptr();
    session->property_->SetCompatibleModeProperty(compatibleModeProperty);
    res = ssm_->GetHostWindowCompatiblityInfo(token, property);
    EXPECT_EQ(WMError::WM_OK, res);
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
} // namespace
} // namespace Rosen
} // namespace OHOS