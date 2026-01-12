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
#include "iremote_object_mocker.h"
#include "interfaces/include/ws_common.h"
#include "screen_fold_data.h"
#include "session_manager/include/scene_session_manager.h"
#include "session_info.h"
#include "session/host/include/scene_session.h"
#include "session/host/include/main_session.h"
#include "window_manager_agent.h"
#include "session_manager.h"
#include "zidl/window_manager_agent_interface.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "mock/mock_accesstoken_kit.h"
#include "application_info.h"
#include "context.h"
#include "get_snapshot_callback.h"

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
class SceneSessionManagerLifecycleTest2 : public testing::Test {
public:
    static void SetUpTestCase();

    static void TearDownTestCase();

    void SetUp() override;

    void TearDown() override;

    static void SetVisibleForAccessibility(sptr<SceneSession>& sceneSession);
    int32_t GetTaskCount(sptr<SceneSession>& session);
    static sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerLifecycleTest2::ssm_ = nullptr;

void WindowChangedFuncTest(int32_t persistentId, WindowUpdateType type) {}

void ProcessStatusBarEnabledChangeFuncTest(bool enable) {}

void SceneSessionManagerLifecycleTest2::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SceneSessionManagerLifecycleTest2::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionManagerLifecycleTest2::SetUp()
{
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerLifecycleTest2::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    ssm_->sceneSessionMap_.clear();
}

void SceneSessionManagerLifecycleTest2::SetVisibleForAccessibility(sptr<SceneSession>& sceneSession)
{
    sceneSession->SetTouchable(true);
    sceneSession->forceTouchable_ = true;
    sceneSession->systemTouchable_ = true;
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    sceneSession->foregroundInteractiveStatus_.store(true);
}

int32_t SceneSessionManagerLifecycleTest2::GetTaskCount(sptr<SceneSession>& session)
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
 * @tc.name: OnSessionStateChange
 * @tc.desc: OnSessionStateChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest2, OnSessionStateChange, TestSize.Level1)
{
    SessionState state = SessionState::STATE_FOREGROUND;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    ssm_->OnSessionStateChange(1, state);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerLifecycleTest22";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ASSERT_NE(nullptr, ssm_);
    ssm_->OnSessionStateChange(1, state);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(1);
    ssm_->OnSessionStateChange(1, state);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(nullptr, ssm_);
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = true;
    ssm_->OnSessionStateChange(1, state);
    ASSERT_NE(nullptr, ssm_);
    ssm_->needBlockNotifyFocusStatusUntilForeground_ = false;
    ssm_->OnSessionStateChange(1, state);
    focusGroup->SetFocusedSessionId(0);
    ASSERT_NE(nullptr, ssm_);
    ssm_->OnSessionStateChange(1, state);
}

/**
 * @tc.name: OnSessionStateChange01
 * @tc.desc: OnSessionStateChange01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest2, OnSessionStateChange01, TestSize.Level1)
{
    SessionState state = SessionState::STATE_BACKGROUND;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerLifecycleTest22";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ASSERT_NE(nullptr, ssm_);
    ssm_->OnSessionStateChange(1, state);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_NE(nullptr, ssm_);
    ssm_->OnSessionStateChange(1, state);
    state = SessionState::STATE_END;
    ASSERT_NE(nullptr, ssm_);
    ssm_->OnSessionStateChange(1, state);
}

/**
 * @tc.name: OnSessionStateChange02
 * @tc.desc: OnSessionStateChange02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest2, OnSessionStateChange02, TestSize.Level1)
{
    SessionState state = SessionState::STATE_FOREGROUND;
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.clear();
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "SceneSessionManagerLifecycleTest22";
    sessionInfo.abilityName_ = "DumpSessionWithId";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    ASSERT_NE(nullptr, ssm_);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    ASSERT_NE(nullptr, sceneSession);
    ASSERT_NE(nullptr, sceneSession->property_);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    sceneSession->SetFocusedOnShow(true);
    ASSERT_NE(nullptr, ssm_);
    ssm_->OnSessionStateChange(1, state);
    sceneSession->SetFocusedOnShow(false);
    ASSERT_NE(nullptr, ssm_);
    ssm_->OnSessionStateChange(1, state);
}

/**
 * @tc.name: MinimizeAllAppWindows
 * @tc.desc: MinimizeAllAppWindows
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest2, MinimizeAllAppWindows, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "MinimizeAllAppWindows";
    info.bundleName_ = "MinimizeAllAppWindows";
    info.screenId_ = 0;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);

    DisplayId displayId = 0;
    int32_t excludeWindowId = 0;
    MockAccesstokenKit::MockIsSACalling(false);
    MockAccesstokenKit::MockIsSystemApp(false);
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId), WMError::WM_ERROR_NOT_SYSTEM_APP);
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId, excludeWindowId), WMError::WM_ERROR_NOT_SYSTEM_APP);

    MockAccesstokenKit::MockIsSACalling(true);
    MockAccesstokenKit::MockIsSystemApp(true);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId), WMError::WM_OK);
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId, excludeWindowId), WMError::WM_OK);

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId), WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId, excludeWindowId), WMError::WM_ERROR_DEVICE_NOT_SUPPORT);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId), WMError::WM_OK);
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId, excludeWindowId), WMError::WM_OK);

    excludeWindowId = 10000;
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId, excludeWindowId), WMError::WM_ERROR_INVALID_OPERATION);
    excludeWindowId = 0;
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId, excludeWindowId), WMError::WM_OK);
}

/**
 * @tc.name: MinimizeAllAppWindows
 * @tc.desc: MinimizeAllAppWindows
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest2, MinimizeAllWindow01, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "MinimizeAllWindow01";
    info.bundleName_ = "MinimizeAllWindow01";
    info.screenId_ = 0;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);

    DisplayId displayId = 0;
    int32_t excludeWindowId = 0;
    MockAccesstokenKit::MockIsSACalling(true);
    MockAccesstokenKit::MockIsSystemApp(true);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->property_ = property;
    sceneSession->SetSessionInfoPersistentId(9);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    sceneSession->SetSessionLabel("MinimizeAllWindow01");
    sceneSession->SetScreenId(0);
    ssm_->sceneSessionMap_.insert({1, nullptr});
    ssm_->sceneSessionMap_.insert({9, sceneSession});
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId), WMError::WM_OK);
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId, excludeWindowId), WMError::WM_OK);
    excludeWindowId = 8;
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId, excludeWindowId), WMError::WM_ERROR_INVALID_OPERATION);
    excludeWindowId = 9;
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId, excludeWindowId), WMError::WM_OK);

    property->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    sceneSession->property_ = property;
    ssm_->sceneSessionMap_.insert({9, sceneSession});
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId), WMError::WM_OK);
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId, excludeWindowId), WMError::WM_OK);

    sceneSession->SetScreenId(1);
    ssm_->sceneSessionMap_.insert({9, sceneSession});
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId), WMError::WM_OK);
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId, excludeWindowId), WMError::WM_OK);

    sceneSession->SetScreenId(1);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    sceneSession->property_ = property;
    ssm_->sceneSessionMap_.insert({9, sceneSession});
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId), WMError::WM_OK);
    EXPECT_EQ(ssm_->MinimizeAllAppWindows(displayId, excludeWindowId), WMError::WM_OK);
}

/**
 * @tc.name: NotifyWindowStateErrorFromMMI
 * @tc.desc: NotifyWindowStateErrorFromMMI
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest2, NotifyWindowStateErrorFromMMI, TestSize.Level1)
{
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "testAbilityName";
    info.bundleName_ = "testBundleName";
    info.screenId_ = 0;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->property_ = property;
    sceneSession->SetScbCoreEnabled(true);
    sceneSession->SetCallingPid(100);

    SessionInfo info1;
    info1.abilityName_ = "testAbilityName1";
    info1.bundleName_ = "testBundleName1";
    info1.screenId_ = 0;
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(info1, nullptr);
    ASSERT_NE(nullptr, sceneSession1);
    sceneSession1->property_ = property;
    sceneSession1->SetScbCoreEnabled(true);
    sceneSession1->SetCallingPid(200);

    SessionInfo info2;
    info2.abilityName_ = "SceneSessionManagerLifecycleTest2";
    info2.bundleName_ = "NotifyWindowStateErrorFromMMI2";
    info2.screenId_ = 0;
    sptr<SceneSession> sceneSession2 = sptr<SceneSession>::MakeSptr(info2, nullptr);
    ASSERT_NE(nullptr, sceneSession2);
    sptr<WindowSessionProperty> property2 = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property2);
    property2->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSession2->property_ = property2;
    sceneSession2->SetCallingPid(100);

    ssm_->sceneSessionMap_.insert({ 10086, sceneSession });
    ssm_->sceneSessionMap_.insert({ 10087, sceneSession1 });
    ssm_->sceneSessionMap_.insert({ 10088, sceneSession2 });
    ssm_->sceneSessionMap_.insert({ 10089, nullptr });

    ssm_->NotifyWindowStateErrorFromMMI(-1, 10086);

    ssm_->NotifyWindowStateErrorFromMMI(100, 10086);
    ssm_->NotifyWindowStateErrorFromMMI(100, 10088);
    
    ssm_->NotifyWindowStateErrorFromMMI(201, 10087);
}

/**
 * @tc.name: GetAllMainWindowInfo
 * @tc.desc: GetAllMainWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest2, GetAllMainWindowInfo, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    ssm_->sceneSessionMap_.clear();
    SessionInfo info;
    info.abilityName_ = "SceneSessionManagerLifecycleTest2";
    info.bundleName_ = "GetAllMainWindowInfo";
    info.screenId_ = 0;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->property_ = property;
    sceneSession->SetSessionInfoPersistentId(99);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    sceneSession->SetSessionLabel("GetAllMainWindowInfo");
 
    std::vector<sptr<MainWindowInfo>> infos;
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(ssm_->GetAllMainWindowInfo(infos), WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(ssm_->GetAllMainWindowInfo(infos), WMError::WM_ERROR_INVALID_PERMISSION);
 
    ssm_->sceneSessionMap_.insert({1, nullptr});
    ssm_->sceneSessionMap_.insert({9, sceneSession});
    MockAccesstokenKit::MockAccessTokenKitRet(0);
    EXPECT_EQ(ssm_->GetAllMainWindowInfo(infos), WMError::WM_OK);
 
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    EXPECT_EQ(ssm_->GetAllMainWindowInfo(infos), WMError::WM_OK);

    infos.clear();
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->SetSessionState(SessionState::STATE_DISCONNECT);
    EXPECT_EQ(ssm_->GetAllMainWindowInfo(infos), WMError::WM_OK);
    EXPECT_EQ(static_cast<int32_t>(infos.size()), 0);
    EXPECT_TRUE(g_logMsg.find("session is nullptr or sessionState is disconnect") != std::string::npos);
}
 
/**
 * @tc.name: GetMainWindowSnapshot
 * @tc.desc: GetMainWindowSnapshot
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest2, GetMainWindowSnapshot, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    SessionInfo info;
    info.abilityName_ = "SceneSessionManagerLifecycleTest2";
    info.bundleName_ = "GetMainWindowSnapshot";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->property_ = property;
    sceneSession->SetCallingPid(100);
    sceneSession->SetSessionInfoPersistentId(9);
    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    sceneSession->SetSessionLabel("GetMainWindowSnapshot");
 
    std::vector<int32_t> windowIds;
    WindowSnapshotConfiguration configs;
    configs.useCache = true;
    sptr<IRemoteObject> callback = sptr<IRemoteObjectMocker>::MakeSptr();
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    EXPECT_EQ(ssm_->GetMainWindowSnapshot(windowIds, configs, callback), WMError::WM_ERROR_DEVICE_NOT_SUPPORT);

    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(ssm_->GetMainWindowSnapshot(windowIds, configs, callback), WMError::WM_ERROR_INVALID_PERMISSION);
    MockAccesstokenKit::MockAccessTokenKitRet(0);
    EXPECT_EQ(ssm_->GetMainWindowSnapshot(windowIds, configs, callback), WMError::WM_ERROR_INVALID_PARAM);

    windowIds.emplace_back(1);
    windowIds.emplace_back(9);
    EXPECT_EQ(ssm_->GetMainWindowSnapshot(windowIds, configs, nullptr), WMError::WM_ERROR_INVALID_PARAM);
    MockAccesstokenKit::MockAccessTokenKitRet(-1);
    EXPECT_EQ(ssm_->GetMainWindowSnapshot(windowIds, configs, callback), WMError::WM_ERROR_INVALID_PERMISSION);
    MockAccesstokenKit::MockAccessTokenKitRet(0);
    ssm_->GetMainWindowSnapshot(windowIds, configs, callback);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(g_logMsg.find("Get snapshot failed") != std::string::npos);

    ssm_->sceneSessionMap_.insert({1, sceneSession});
    ssm_->sceneSessionMap_.insert({9, sceneSession});
    sptr<GetSnapshotCallback> getSnapshotCallback = sptr<GetSnapshotCallback>::MakeSptr();
    EXPECT_EQ(ssm_->GetMainWindowSnapshot(windowIds, configs, getSnapshotCallback), WMError::WM_OK);

    configs.useCache = true;
    sptr<IRemoteObject> callbackNull = sptr<IRemoteObjectMocker>::MakeSptr();
    EXPECT_EQ(ssm_->GetMainWindowSnapshot(windowIds, configs, callbackNull), WMError::WM_OK);

    property->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    sceneSession->property_ = property;
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert({1, sceneSession});
    ssm_->sceneSessionMap_.insert({9, sceneSession});
    ssm_->GetMainWindowSnapshot(windowIds, configs, callback);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(g_logMsg.find("is not mainWindow") != std::string::npos);
}
 
/**
 * @tc.name: GetMainWindowSnapshot01
 * @tc.desc: GetMainWindowSnapshot01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerLifecycleTest2, GetMainWindowSnapshot01, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    SessionInfo info;
    info.abilityName_ = "SceneSessionManagerLifecycleTest2";
    info.bundleName_ = "GetMainWindowSnapshot01";
    info.screenId_ = 0;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(nullptr, property);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession->property_ = property;
    sceneSession->SetCallingPid(99);
 
    std::vector<int32_t> windowIdsRepeat;
    windowIdsRepeat.emplace_back(1);
    windowIdsRepeat.emplace_back(1);
    WindowSnapshotConfiguration configs;
    configs.useCache = true;
    MockAccesstokenKit::MockAccessTokenKitRet(0);
    sptr<IRemoteObject> callback = sptr<IRemoteObjectMocker>::MakeSptr();
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    EXPECT_EQ(ssm_->GetMainWindowSnapshot(windowIdsRepeat, configs, callback), WMError::WM_ERROR_INVALID_PARAM);
 
    std::vector<int32_t> windowIdsAbnormal;
    windowIdsAbnormal.emplace_back(1); // windowId
    windowIdsAbnormal.emplace_back(1000); // abnormal windowId
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    MockAccesstokenKit::MockAccessTokenKitRet(0);
    ssm_->GetMainWindowSnapshot(windowIdsAbnormal, configs, callback);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_TRUE(g_logMsg.find("Get snapshot failed") != std::string::npos);
 
    std::vector<int32_t> windowIdsMaxSize;
    ssm_->sceneSessionMap_.clear();
    for (int i = 0; i < 600; i++) { // windowIdsMax size
        windowIdsMaxSize.emplace_back(i);
    }
    callback = sceneSession->AsObject();
    EXPECT_EQ(ssm_->GetMainWindowSnapshot(windowIdsMaxSize, configs, callback), WMError::WM_ERROR_INVALID_PARAM);
}

} // namespace
} // namespace Rosen
} // namespace OHOS
