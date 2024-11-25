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

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

sptr<SceneSessionManager> SceneSessionManagerTest5::ssm_ = nullptr;
bool SceneSessionManagerTest5::gestureNavigationEnabled_ = true;

ProcessGestureNavigationEnabledChangeFunc SceneSessionManagerTest5::callbackFunc_ = [](bool enable,
    const std::string& bundleName, GestureBackType type) {
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
}

void SceneSessionManagerTest5::TearDown()
{
    ssm_->sceneSessionMap_.clear();
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name: NotifySessionTouchOutside
 * @tc.desc: SceneSesionManager notify session touch outside
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, NotifySessionTouchOutside01, Function | SmallTest | Level3)
{
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(ssm_, nullptr);
    ssm_->recoveringFinished_ = false;
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, property);
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    ssm_->NotifySessionTouchOutside(0);
    property->SetPersistentId(1);
    ssm_->NotifySessionTouchOutside(1);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSessionInner
 * @tc.desc: check func DestroyAndDisconnectSpecificSessionInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, DestroyAndDisconnectSpecificSessionInner, Function | SmallTest | Level2)
{
    sptr<ISession> session;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(nullptr, property);
    std::vector<int32_t> recoveredPersistentIds = {0, 1, 2};
    ssm_->SetAlivePersistentIds(recoveredPersistentIds);
    ProcessShiftFocusFunc shiftFocusFunc_;
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ssm_->DestroyAndDisconnectSpecificSessionInner(1);
    property->SetPersistentId(1);
    ssm_->DestroyAndDisconnectSpecificSessionInner(1);

    property->SetWindowType(WindowType::WINDOW_TYPE_TOAST);
    ssm_->DestroyAndDisconnectSpecificSessionInner(1);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSessionWithDetachCallback
 * @tc.desc: SceneSesionManager destroy and disconnect specific session with detach callback
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, DestroyAndDetachCallback, Function | SmallTest | Level3)
{
    int32_t persistentId = 0;
    ASSERT_NE(ssm_, nullptr);
    sptr<IRemoteObject> callback = new (std::nothrow) IRemoteObjectMocker();
    ASSERT_NE(callback, nullptr);
    ssm_->DestroyAndDisconnectSpecificSessionWithDetachCallback(persistentId, callback);
    sptr<WindowSessionProperty> property;
    ssm_->recoveringFinished_ = false;
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, property);
    ssm_->DestroyAndDisconnectSpecificSessionWithDetachCallback(persistentId, callback);
}

/**
 * @tc.name: GetStartupPageFromResource
 * @tc.desc: GetStartupPageFromResource
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, GetStartupPageFromResource, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    AppExecFwk::AbilityInfo info;
    info.startWindowBackgroundId = 1;
    std::string path = "path";
    uint32_t bgColor = 1;
    ASSERT_EQ(false, ssm_->GetStartupPageFromResource(info, path, bgColor));
    info.startWindowIconId = 0;
    ASSERT_EQ(false, ssm_->GetStartupPageFromResource(info, path, bgColor));
    info.hapPath = "hapPath";
    ASSERT_EQ(false, ssm_->GetStartupPageFromResource(info, path, bgColor));
}

/**
 * @tc.name: GetStartupPage01
 * @tc.desc: GetStartupPage when resourceMgr is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, GetStartupPage01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    /**
     * @tc.steps: step1. Build input parameter.
     */
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    std::string path;
    uint32_t bgColor = 0x00000000;

    /**
     * @tc.steps: step2. Set bundleMgr_ to nullptr.
     */
    sptr<AppExecFwk::IBundleMgr> tempBundleMgr = ssm_->bundleMgr_;
    ssm_->bundleMgr_ = nullptr;

    /**
     * @tc.steps: step3. Test and check result.
     */
    ssm_->GetStartupPage(info, path, bgColor);
    ssm_->bundleMgr_ = tempBundleMgr;
    EXPECT_EQ("", path);
    EXPECT_EQ(0x00000000, bgColor);
}

/**
 * @tc.name: GetStartupPage02
 * @tc.desc: GetStartupPage when info is cached
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, GetStartupPage02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    /**
     * @tc.steps: step1. Build input parameter.
     */
    SessionInfo sessionInfo;
    sessionInfo.moduleName_ = "moduleName";
    sessionInfo.abilityName_ = "abilityName";
    sessionInfo.bundleName_ = "bundleName";
    uint32_t bgColor = 0x00000000;
    std::string path;

    /**
     * @tc.steps: step2. Cache info.
     */
    const uint32_t cachedColor = 0xff000000;
    const std::string cachedPath = "cachedPath";
    auto key = sessionInfo.moduleName_ + sessionInfo.abilityName_;
    StartingWindowInfo startingWindowInfo = {
        .startingWindowBackgroundId_ = 0,
        .startingWindowIconId_ = 0,
        .startingWindowBackgroundColor_ = cachedColor,
        .startingWindowIconPath_ = cachedPath,
    };
    std::map<std::string, StartingWindowInfo> startingWindowInfoMap{{ key, startingWindowInfo }};
    ssm_->startingWindowMap_.insert({sessionInfo.bundleName_, startingWindowInfoMap});

    /**
     * @tc.steps: step3. Test and check result.
     */
    ssm_->GetStartupPage(sessionInfo, path, bgColor);
    EXPECT_EQ(path, cachedPath);
    EXPECT_EQ(bgColor, cachedColor);
}

/**
 * @tc.name: CacheStartingWindowInfo01
 * @tc.desc: Cache new starting window info
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CacheStartingWindowInfo01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->startingWindowMap_.clear();
    /**
     * @tc.steps: step1. Build input parameter.
     */
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.name = "abilityName";
    abilityInfo.bundleName = "bundleName";
    abilityInfo.moduleName = "moduleName";
    abilityInfo.startWindowBackgroundId = 1;
    abilityInfo.startWindowIconId = 2;
    std::string path = "cachedPath";
    uint32_t bgColor = 0xff000000;

    /**
     * @tc.steps: step2. Cache info and check result.
     */
    ssm_->CacheStartingWindowInfo(abilityInfo, path, bgColor);
    auto iter = ssm_->startingWindowMap_.find(abilityInfo.bundleName);
    ASSERT_NE(iter, ssm_->startingWindowMap_.end());
    auto& infoMap = iter->second;
    auto infoIter = infoMap.find(abilityInfo.moduleName + abilityInfo.name);
    ASSERT_NE(infoIter, infoMap.end());
    EXPECT_EQ(infoIter->second.startingWindowBackgroundId_, 1);
    EXPECT_EQ(infoIter->second.startingWindowIconId_, 2);
    EXPECT_EQ(infoIter->second.startingWindowBackgroundColor_, bgColor);
    EXPECT_EQ(infoIter->second.startingWindowIconPath_, path);
}

/**
 * @tc.name: CacheStartingWindowInfo02
 * @tc.desc: Execute when info is cached
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CacheStartingWindowInfo02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->startingWindowMap_.clear();
    /**
     * @tc.steps: step1. Build input parameter.
     */
    AppExecFwk::AbilityInfo abilityInfo;
    abilityInfo.name = "abilityName";
    abilityInfo.bundleName = "bundleName";
    abilityInfo.moduleName = "moduleName";
    abilityInfo.startWindowBackgroundId = 1;
    abilityInfo.startWindowIconId = 2;
    std::string path = "cachedPath";
    uint32_t bgColor = 0xff000000;

    /**
     * @tc.steps: step2. Insert one item.
     */
    auto key = abilityInfo.moduleName + abilityInfo.name;
    StartingWindowInfo startingWindowInfo = {
        .startingWindowBackgroundId_ = 0,
        .startingWindowIconId_ = 0,
        .startingWindowBackgroundColor_ = 0x00000000,
        .startingWindowIconPath_ = "path",
    };
    std::map<std::string, StartingWindowInfo> startingWindowInfoMap{{ key, startingWindowInfo }};
    ssm_->startingWindowMap_.insert({abilityInfo.bundleName, startingWindowInfoMap});

    /**
     * @tc.steps: step3. Execute and check result.
     */
    ssm_->CacheStartingWindowInfo(abilityInfo, path, bgColor);
    auto iter = ssm_->startingWindowMap_.find(abilityInfo.bundleName);
    ASSERT_NE(iter, ssm_->startingWindowMap_.end());
    auto& infoMap = iter->second;
    auto infoIter = infoMap.find(abilityInfo.moduleName + abilityInfo.name);
    ASSERT_NE(infoIter, infoMap.end());
    EXPECT_NE(infoIter->second.startingWindowBackgroundId_, 1);
    EXPECT_NE(infoIter->second.startingWindowIconId_, 2);
    EXPECT_NE(infoIter->second.startingWindowBackgroundColor_, bgColor);
    EXPECT_NE(infoIter->second.startingWindowIconPath_, path);
}

/**
 * @tc.name: OnBundleUpdated
 * @tc.desc: Erase cached info when bundle update
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, OnBundleUpdated, Function | SmallTest | Level3)
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
    uint32_t cachedColor = 0xff000000;
    std::string cachedPath = "cachedPath";
    auto key = sessionInfo.moduleName_ + sessionInfo.abilityName_;
    StartingWindowInfo startingWindowInfo = {
        .startingWindowBackgroundId_ = 0,
        .startingWindowIconId_ = 0,
        .startingWindowBackgroundColor_ = cachedColor,
        .startingWindowIconPath_ = cachedPath,
    };
    std::map<std::string, StartingWindowInfo> startingWindowInfoMap{{ key, startingWindowInfo }};
    ssm_->startingWindowMap_.insert({sessionInfo.bundleName_, startingWindowInfoMap});
    ASSERT_NE(ssm_->startingWindowMap_.size(), 0);

    /**
     * @tc.steps: step2. On bundle updated and check map.
     */
    ssm_->OnBundleUpdated(sessionInfo.bundleName_, 0);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(ssm_->startingWindowMap_.size(), 0);
}

/**
 * @tc.name: OnConfigurationUpdated
 * @tc.desc: Clear startingWindowMap when configuration update
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, OnConfigurationUpdated, Function | SmallTest | Level3)
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
    uint32_t cachedColor = 0xff000000;
    std::string cachedPath = "cachedPath";
    auto key = sessionInfo.moduleName_ + sessionInfo.abilityName_;
    StartingWindowInfo startingWindowInfo = {
        .startingWindowBackgroundId_ = 0,
        .startingWindowIconId_ = 0,
        .startingWindowBackgroundColor_ = cachedColor,
        .startingWindowIconPath_ = cachedPath,
    };
    std::map<std::string, StartingWindowInfo> startingWindowInfoMap{{ key, startingWindowInfo }};
    ssm_->startingWindowMap_.insert({sessionInfo.bundleName_, startingWindowInfoMap});
    ASSERT_NE(ssm_->startingWindowMap_.size(), 0);

    /**
     * @tc.steps: step2. On configuration updated and check map.
     */
    auto configuration = std::make_shared<AppExecFwk::Configuration>();
    ssm_->OnConfigurationUpdated(configuration);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(ssm_->startingWindowMap_.size(), 0);
}

/**
 * @tc.name: CreateKeyboardPanelSession
 * @tc.desc: CreateKeyboardPanelSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CreateKeyboardPanelSession, Function | SmallTest | Level3)
{
    SceneSessionManager* sceneSessionManager = new SceneSessionManager();
    ASSERT_NE(sceneSessionManager, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::SYSTEM_SUB_WINDOW_BASE);
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    sceneSessionManager->CreateKeyboardPanelSession(sceneSession);
    delete sceneSessionManager;
}

/**
 * @tc.name: PrepareTerminate
 * @tc.desc: SceneSesionManager prepare terminate
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, PrepareTerminate, Function | SmallTest | Level3)
{
    int32_t persistentId = 1;
    bool isPrepareTerminate = true;
    SceneSessionManager* sceneSessionManager = new SceneSessionManager();
    ASSERT_NE(sceneSessionManager, nullptr);
    ASSERT_EQ(WSError::WS_OK, sceneSessionManager->PrepareTerminate(persistentId, isPrepareTerminate));
    delete sceneSessionManager;
}

/**
 * @tc.name: IsKeyboardForeground
 * @tc.desc: IsKeyboardForeground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, IsKeyboardForeground, Function | SmallTest | Level3)
{
    SceneSessionManager* sceneSessionManager = new SceneSessionManager();
    ASSERT_NE(sceneSessionManager, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    sceneSessionManager->IsKeyboardForeground();
    property->SetWindowType(WindowType::WINDOW_TYPE_SYSTEM_ALARM_WINDOW);
    sceneSessionManager->IsKeyboardForeground();
    delete sceneSessionManager;
}

/**
 * @tc.name: RequestInputMethodCloseKeyboard
 * @tc.desc: RequestInputMethodCloseKeyboard
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestInputMethodCloseKeyboard02, Function | SmallTest | Level3)
{
    int32_t persistentId = -1;
    bool isPrepareTerminate = true;
    SceneSessionManager* sceneSessionManager = new SceneSessionManager();
    ASSERT_NE(sceneSessionManager, nullptr);
    sceneSessionManager->PrepareTerminate(persistentId, isPrepareTerminate);
    delete sceneSessionManager;
}

/**
 * @tc.name: HandleSpecificSystemBarProperty
 * @tc.desc: HandleSpecificSystemBarProperty
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, HandleSpecificSystemBarProperty, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    WindowType type = WindowType::WINDOW_TYPE_STATUS_BAR;
    ssm_->HandleSpecificSystemBarProperty(type, property, sceneSession);
}

/**
 * @tc.name: UpdateBrightness
 * @tc.desc: UpdateBrightness
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, UpdateBrightness, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.isSystem_ = false;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    sptr<SceneSession> sceneSession = ssm_->CreateSceneSession(info, property);
    ASSERT_NE(property, nullptr);
    ssm_->UpdateBrightness(1);
    FocusChangeInfo focusInfo;
    ssm_->GetCurrentUserId();
    ssm_->GetFocusWindowInfo(focusInfo);
}

/**
 * @tc.name: RegisterSessionSnapshotFunc
 * @tc.desc: RegisterSessionSnapshotFunc
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RegisterSessionSnapshotFunc, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->RegisterSessionSnapshotFunc(sceneSession);
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    sptr<SceneSession> sceneSession1 = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    ssm_->RegisterSessionSnapshotFunc(sceneSession);
    info.isSystem_ = false;
    ssm_->RegisterSessionSnapshotFunc(sceneSession);
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo;
    ssm_->RegisterSessionSnapshotFunc(sceneSession);
}

/**
 * @tc.name: RequestAllAppSessionUnfocus
 * @tc.desc: RequestAllAppSessionUnfocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestAllAppSessionUnfocus, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->HandleHideNonSystemFloatingWindows(property, sceneSession);
    ssm_->RequestAllAppSessionUnfocus();
}

/**
 * @tc.name: RequestFocusStatus
 * @tc.desc: RequestFocusStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestFocusStatus, Function | SmallTest | Level3)
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
HWTEST_F(SceneSessionManagerTest5, RequestFocusStatus01, Function | SmallTest | Level3)
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
HWTEST_F(SceneSessionManagerTest5, RequestSessionFocus, Function | SmallTest | Level3)
{
    FocusChangeReason reason = FocusChangeReason::DEFAULT;
    WSError ret = ssm_->RequestSessionFocus(0, true, reason);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
    ret = ssm_->RequestSessionFocus(100, true, reason);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);

    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetFocusable(false);
    sptr<SceneSession> sceneSession1 = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);

    sceneSession1->property_ = property;
    ASSERT_NE(sceneSession1->property_, nullptr);
    sceneSession1->persistentId_ = 1;
    sceneSession1->isVisible_ = true;
    sceneSession1->state_ = SessionState::STATE_ACTIVE;
    sceneSession1->focusedOnShow_ = true;
    sceneSession1->property_->focusable_ = true;
    sceneSession1->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->focusedSessionId_ = 2;
    ssm_->sceneSessionMap_.insert({sceneSession1->GetPersistentId(), sceneSession1});
    ret = ssm_->RequestSessionFocus(1, true, reason);
    ASSERT_EQ(ret, WSError::WS_OK);
    ASSERT_EQ(ssm_->focusedSessionId_, 1);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: RequestFocusClient
 * @tc.desc: RequestFocusClient
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestFocusClient, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.abilityName_ = "RequestFocusTest1";
    info.bundleName_ = "RequestFocusTest1";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetFocusable(true);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_ = property;
    sceneSession->persistentId_ = 1;
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    sceneSession->SetZOrder(1);

    SessionInfo info2;
    info2.abilityName_ = "RequestFocusTest2";
    info2.bundleName_ = "RequestFocusTest2";
    sptr<SceneSession> sceneSession2 = nullptr;
    sptr<WindowSessionProperty> property2 = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property2, nullptr);
    property2->SetFocusable(true);
    property2->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession2 = new (std::nothrow) SceneSession(info2, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    sceneSession2->property_ = property2;
    sceneSession2->persistentId_ = 2;
    sceneSession2->isVisible_ = true;
    sceneSession2->state_ = SessionState::STATE_ACTIVE;
    sceneSession2->SetZOrder(2);
    ssm_->sceneSessionMap_.insert({sceneSession->GetPersistentId(), sceneSession});
    ssm_->sceneSessionMap_.insert({sceneSession2->GetPersistentId(), sceneSession2});
    FocusChangeReason reason = FocusChangeReason::CLIENT_REQUEST;

    ssm_->RequestSessionFocus(1, false, reason);
    ASSERT_EQ(ssm_->focusedSessionId_, 1);
    ssm_->RequestSessionFocus(2, false, reason);
    ASSERT_EQ(ssm_->focusedSessionId_, 2);
    ssm_->RequestSessionUnfocus(2, reason);
    ASSERT_EQ(ssm_->focusedSessionId_, 1);
    ssm_->RequestSessionUnfocus(1, reason);
    ASSERT_EQ(ssm_->focusedSessionId_, 0);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: RequestFocusClient
 * @tc.desc: RequestFocusClient
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestFocusClient01, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.abilityName_ = "RequestFocusTest1";
    info.bundleName_ = "RequestFocusTest1";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetFocusable(true);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sceneSession->property_ = property;
    sceneSession->persistentId_ = 1;
    sceneSession->isVisible_ = true;
    sceneSession->state_ = SessionState::STATE_ACTIVE;
    sceneSession->SetZOrder(1);

    SessionInfo info2;
    info2.abilityName_ = "RequestFocusTest2";
    info2.bundleName_ = "RequestFocusTest2";
    sptr<SceneSession> sceneSession2 = nullptr;
    sptr<WindowSessionProperty> property2 = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property2, nullptr);
    property2->SetFocusable(true);
    property2->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession2 = new (std::nothrow) SceneSession(info2, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    sceneSession2->property_ = property2;
    sceneSession2->persistentId_ = 2;
    sceneSession2->isVisible_ = true;
    sceneSession2->state_ = SessionState::STATE_ACTIVE;
    sceneSession2->SetZOrder(2);

    SessionInfo info3;
    info3.abilityName_ = "RequestFocusTest3";
    info3.bundleName_ = "RequestFocusTest3";
    sptr<SceneSession> sceneSession3 = nullptr;
    sptr<WindowSessionProperty> property3 = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property3, nullptr);
    property3->SetFocusable(true);
    property3->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sceneSession3 = new (std::nothrow) SceneSession(info3, nullptr);
    ASSERT_NE(sceneSession3, nullptr);
    sceneSession3->property_ = property3;
    sceneSession3->persistentId_ = 3;
    sceneSession3->isVisible_ = true;
    sceneSession3->state_ = SessionState::STATE_ACTIVE;
    sceneSession3->SetZOrder(3);
    sceneSession3->blockingFocus_ = true;
    ssm_->sceneSessionMap_.insert({sceneSession->GetPersistentId(), sceneSession});
    ssm_->sceneSessionMap_.insert({sceneSession2->GetPersistentId(), sceneSession2});
    ssm_->sceneSessionMap_.insert({sceneSession3->GetPersistentId(), sceneSession3});
    FocusChangeReason reason = FocusChangeReason::CLIENT_REQUEST;

    ssm_->RequestSessionFocus(1, false, reason);
    ASSERT_EQ(ssm_->focusedSessionId_, 1);
    ssm_->RequestSessionFocus(3, false, reason);
    ASSERT_EQ(ssm_->focusedSessionId_, 3);
    ssm_->RequestSessionFocus(2, false, reason);
    ASSERT_EQ(ssm_->focusedSessionId_, 2);
    auto ret = ssm_->RequestSessionUnfocus(3, reason);
    ASSERT_EQ(WSError::WS_DO_NOTHING, ret);
    ssm_->RequestSessionUnfocus(2, reason);
    ASSERT_EQ(ssm_->focusedSessionId_, 1);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: SetShiftFocusListener
 * @tc.desc: SetShiftFocusListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, SetShiftFocusListener, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    FocusChangeReason reason = FocusChangeReason::SPLIT_SCREEN;
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->ShiftFocus(sceneSession, reason);
    info.isSystem_ = true;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    sptr<SceneSession> sceneSession1 = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    ProcessShiftFocusFunc fun;
    NotifySCBAfterUpdateFocusFunc func;
    ssm_->SetShiftFocusListener(fun);
    ssm_->SetSCBFocusedListener(func);
    ssm_->SetSCBUnfocusedListener(func);
    ProcessCallingSessionIdChangeFunc func1;
    ssm_->SetCallingSessionIdSessionListenser(func1);
    ProcessStartUIAbilityErrorFunc func2;
    ssm_->SetStartUIAbilityErrorListener(func2);
    ssm_->ShiftFocus(sceneSession1, reason);
}

/**
 * @tc.name: UpdateFocusStatus
 * @tc.desc: UpdateFocusStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, UpdateFocusStatus, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->UpdateFocusStatus(sceneSession, false);
    ssm_->UpdateFocusStatus(sceneSession, true);

    sptr<SceneSession> sceneSession1 = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    ssm_->UpdateFocusStatus(sceneSession1, true);
    ssm_->UpdateFocusStatus(sceneSession1, false);
}

/**
 * @tc.name: RequestSessionUnfocus
 * @tc.desc: RequestSessionUnfocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestSessionUnfocus, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    sptr<SceneSession> sceneSession1 = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    FocusChangeReason reason = FocusChangeReason::MOVE_UP;
    WSError ret = ssm_->RequestSessionUnfocus(0, reason);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);

    sptr<SceneSession> focusedSession = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(focusedSession, nullptr);
    focusedSession->property_ = property;
    ASSERT_NE(focusedSession->property_, nullptr);
    sceneSession1->persistentId_ = 1;
    focusedSession->persistentId_ = 2;
    focusedSession->property_->parentPersistentId_ = 1;
    ssm_->focusedSessionId_ = 1;
    ssm_->sceneSessionMap_.insert({sceneSession1->GetPersistentId(), sceneSession1});
    ssm_->sceneSessionMap_.insert({focusedSession->GetPersistentId(), focusedSession});
    ret = ssm_->RequestSessionUnfocus(1, reason);
    ASSERT_EQ(ret, WSError::WS_OK);
    ASSERT_NE(ssm_->focusedSessionId_, 1);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: RequestFocusSpecificCheck
 * @tc.desc: RequestFocusSpecificCheck
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestFocusSpecificCheck, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    sptr<SceneSession> sceneSession1 = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    FocusChangeReason reason = FocusChangeReason::MOVE_UP;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    WSError ret = ssm_->RequestFocusSpecificCheck(sceneSession1, true, reason);
    EXPECT_EQ(ret, WSError::WS_OK);
    FocusChangeReason reason1 = FocusChangeReason::SPLIT_SCREEN;
    property->SetWindowType(WindowType::APP_WINDOW_BASE);
    ret = ssm_->RequestFocusSpecificCheck(sceneSession1, true, reason1);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: NotifyFocusStatus
 * @tc.desc: NotifyFocusStatus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, NotifyFocusStatus, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->NotifyFocusStatus(sceneSession, true);
    sptr<SceneSession> sceneSession1 = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    ssm_->NotifyFocusStatus(sceneSession1, false);
    info.isSystem_ = true;
    ssm_->NotifyFocusStatus(sceneSession1, true);
}

/**
 * @tc.name: NotifyFocusStatusByMission
 * @tc.desc: NotifyFocusStatusByMission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, NotifyFocusStatusByMission, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<SceneSession> currSession = nullptr;
    ssm_->NotifyFocusStatusByMission(sceneSession, currSession);
    ASSERT_EQ(false, ssm_->MissionChanged(sceneSession, currSession));
    sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->NotifyFocusStatusByMission(sceneSession, currSession);
    ssm_->MissionChanged(sceneSession, currSession);
    currSession = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(currSession, nullptr);
    ssm_->NotifyFocusStatusByMission(sceneSession, currSession);
    ssm_->MissionChanged(sceneSession, currSession);
    info.isSystem_ = true;
    ssm_->NotifyFocusStatusByMission(sceneSession, currSession);
    ssm_->MissionChanged(sceneSession, currSession);
}

/**
 * @tc.name: UpdateFocus
 * @tc.desc: UpdateFocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, UpdateFocus, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    sptr<SceneSession> sceneSession1 = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    ssm_->UpdateFocus(1, true);
    ssm_->UpdateFocus(0, false);
}

/**
 * @tc.name: RequestSessionFocusImmediately
 * @tc.desc: RequestSessionFocusImmediately
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestSessionFocusImmediately, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    WSError ret = ssm_->RequestSessionFocusImmediately(0);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
    ssm_->RequestFocusBasicCheck(0);
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    sptr<Session> session = new (std::nothrow) Session(info);
    session->SetSessionProperty(property);
    sptr<SceneSession> sceneSession1 = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    ret = ssm_->RequestSessionFocusImmediately(1);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
    ssm_->RequestFocusBasicCheck(1);
}

/**
 * @tc.name: RequestFocusBasicCheck
 * @tc.desc: RequestFocusBasicCheck
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestFocusBasicCheck, Function | SmallTest | Level3)
{
    ssm_->focusedSessionId_ = 1;
    WSError ret = ssm_->RequestFocusBasicCheck(0);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_SESSION);
    ret = ssm_->RequestFocusBasicCheck(1);
    ASSERT_EQ(ret, WSError::WS_DO_NOTHING);
    ret = ssm_->RequestFocusBasicCheck(2);
    ASSERT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: CheckFocusIsDownThroughBlockingType
 * @tc.desc: CheckFocusIsDownThroughBlockingType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CheckFocusIsDownThroughBlockingType, Function | SmallTest | Level3)
{
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test1";
    SessionInfo info2;
    info2.abilityName_ = "test2";
    info2.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    sptr<SceneSession> sceneSession1 = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession1, nullptr);
    sptr<SceneSession> sceneSession2 = new (std::nothrow) SceneSession(info2, nullptr);
    ASSERT_NE(sceneSession2, nullptr);
    bool ret = ssm_->CheckFocusIsDownThroughBlockingType(sceneSession1, sceneSession2, true);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: CheckFocusIsDownThroughBlockingType
 * @tc.desc: CheckFocusIsDownThroughBlockingType
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CheckFocusIsDownThroughBlockingType01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> requestSceneSession = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(requestSceneSession, nullptr);
    sptr<SceneSession> focusedSession = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(focusedSession, nullptr);
    bool includingAppSession = true;
    bool ret = ssm_->CheckFocusIsDownThroughBlockingType(requestSceneSession, focusedSession, includingAppSession);
    ASSERT_EQ(ret, false);

    requestSceneSession->SetZOrder(0);
    focusedSession->SetZOrder(1);
    focusedSession->blockingFocus_ = true;
    focusedSession->state_ = SessionState::STATE_FOREGROUND;
    focusedSession->isVisible_ = true;
    ssm_->sceneSessionMap_.insert({0, requestSceneSession});
    ssm_->sceneSessionMap_.insert({1, focusedSession});
    ret = ssm_->CheckFocusIsDownThroughBlockingType(requestSceneSession, focusedSession, includingAppSession);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: CheckTopmostWindowFocus
 * @tc.desc: CheckTopmostWindowFocus
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CheckTopmostWindowFocus, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";

    sptr<SceneSession> focusedSession = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(focusedSession, nullptr);
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);

    bool ret = ssm_->CheckTopmostWindowFocus(focusedSession, sceneSession);
    ASSERT_EQ(ret, false);

    focusedSession = new (std::nothrow) MainSession(info, nullptr);
    focusedSession->SetTopmost(true);
    ret = ssm_->CheckTopmostWindowFocus(focusedSession, sceneSession);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: CheckRequestFocusImmdediately
 * @tc.desc: CheckRequestFocusImmdediately
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CheckRequestFocusImmdediately, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";

    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    bool ret = ssm_->CheckRequestFocusImmdediately(sceneSession);
    ASSERT_EQ(ret, false);

    sptr<Session> session = new (std::nothrow) Session(info);
    session->persistentId_ = 1;
    sceneSession->dialogVec_.push_back(session);
    ssm_->focusedSessionId_ = 1;
    ret = ssm_->CheckRequestFocusImmdediately(sceneSession);
    ASSERT_EQ(ret, true);
}

/**
 * @tc.name: GetNextFocusableSession
 * @tc.desc: GetNextFocusableSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, GetNextFocusableSession, Function | SmallTest | Level3)
{
    int32_t persistentId = 0;
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";

    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<SceneSession> result = ssm_->GetNextFocusableSession(persistentId);
    ASSERT_EQ(result, nullptr);

    sceneSession->SetForceHideState(ForceHideState::HIDDEN_WHEN_FOCUSED);
    ssm_->sceneSessionMap_.insert(std::make_pair(1, sceneSession));
    persistentId = 1;
    result = ssm_->GetNextFocusableSession(persistentId);
    ASSERT_EQ(result, nullptr);
}

/**
 * @tc.name: GetTopNearestBlockingFocusSession
 * @tc.desc: GetTopNearestBlockingFocusSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, GetTopNearestBlockingFocusSession, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";

    ssm_->GetTopNearestBlockingFocusSession(2, true);
    sptr<SceneSession> sceneSession = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    ssm_->GetTopNearestBlockingFocusSession(0, true);

    Session session = Session(info);
    session.property_ = nullptr;
    ssm_->GetTopNearestBlockingFocusSession(0, true);
    sptr<SceneSession> session_ = nullptr;
    ssm_->GetTopNearestBlockingFocusSession(0, true);
    session_ = new (std::nothrow) SceneSession(info, nullptr);
    ASSERT_NE(session_, nullptr);
    ssm_->GetTopNearestBlockingFocusSession(0, true);
}

/**
 * @tc.name: PreloadInLakeApp、UpdateSessionAvoidAreaListener
 * @tc.desc: PreloadInLakeApp、UpdateSessionAvoidAreaListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, PreloadInLakeApp, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    int32_t persistentId = 0;

    ssm_->PreloadInLakeApp("");
    sptr<SceneSession> sceneSession = nullptr;

    ssm_->UpdateSessionAvoidAreaListener(persistentId, true);
    sceneSession = ssm_->CreateSceneSession(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({sceneSession->GetPersistentId(), sceneSession});
    ssm_->UpdateSessionAvoidAreaListener(persistentId, true);
}

/**
 * @tc.name: NotifyMMIWindowPidChange
 * @tc.desc: NotifyMMIWindowPidChange
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, NotifyMMIWindowPidChange, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->NotifyMMIWindowPidChange(0, true);
    sceneSession = ssm_->CreateSceneSession(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->sceneSessionMap_.insert({sceneSession->GetPersistentId(), sceneSession});
}

/**
 * @tc.name: ConfigDecor
 * @tc.desc: SceneSesionManager config decor
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, ConfigDecor02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    WindowSceneConfig::ConfigItem* item = new WindowSceneConfig::ConfigItem;
    ASSERT_NE(item, nullptr);
    ssm_->ConfigDecor(*item, false);
    delete item;
}

/**
 * @tc.name: SetSkipSelfWhenShowOnVirtualScreen
 * @tc.desc: SetSkipSelfWhenShowOnVirtualScreen
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, SetSkipSelfWhenShowOnVirtualScreen, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    uint64_t surfaceNodeId = 1234;
    ssm_->SetSkipSelfWhenShowOnVirtualScreen(surfaceNodeId, false);
    ssm_->SetSkipSelfWhenShowOnVirtualScreen(surfaceNodeId, true);
}

/**
 * @tc.name: CreateKeyboardPanelSession
 * @tc.desc: CreateKeyboardPanelSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CreateKeyboardPanelSession02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.screenId_ = SCREEN_ID_INVALID;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->CreateKeyboardPanelSession(sceneSession);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->CreateKeyboardPanelSession(sceneSession);
}

/**
 * @tc.name: InitSceneSession02
 * @tc.desc: InitSceneSession02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, InitSceneSession02, Function | SmallTest | Level3)
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
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(nullptr, property);
    sceneSession = new (std::nothrow) SceneSession(sessionInfo, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->SetSessionProperty(property);

    ssm_->InitSceneSession(sceneSession, sessionInfo, nullptr);
    ASSERT_EQ(DISPLAY_ID_INVALID, sceneSession->GetSessionInfo().screenId_);
}

/**
 * @tc.name: PrepareTerminate
 * @tc.desc: SceneSesionManager prepare terminate
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, PrepareTerminate03, Function | SmallTest | Level3)
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
 * @tc.name: RequestSceneSessionBackground
 * @tc.desc: RequestSceneSessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestSceneSessionBackground, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.ancoSceneState = 0;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
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
 * @tc.name: AddClientDeathRecipient
 * @tc.desc: SceneSesionManager add client death recipient
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, AddClientDeathRecipient02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "AddClientDeathRecipient";
    info.bundleName_ = "AddClientDeathRecipient";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    ssm_->AddClientDeathRecipient(nullptr, sceneSession);
    sptr<ISessionStage> sessionStage = new (std::nothrow) SessionStageMocker();
    ssm_->AddClientDeathRecipient(sessionStage, sceneSession);
}

/**
 * @tc.name: CheckModalSubWindowPermission
 * @tc.desc: CheckModalSubWindowPermission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CheckModalSubWindowPermission, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property->SetWindowFlags(123);
    ssm_->CheckModalSubWindowPermission(property);
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ssm_->CheckModalSubWindowPermission(property);
}

/**
 * @tc.name: CheckSessionPropertyOnRecovery
 * @tc.desc: CheckSessionPropertyOnRecovery
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CheckSessionPropertyOnRecovery, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_UI_EXTENSION);
    property->SetWindowFlags(123);
    WSError result = ssm_->CheckSessionPropertyOnRecovery(property, false);
    ASSERT_EQ(result, WSError::WS_ERROR_NOT_SYSTEM_APP);

    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property->SetParentPersistentId(111);
    result = ssm_->CheckSessionPropertyOnRecovery(property, true);
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_PARAM);
    ssm_->CheckSessionPropertyOnRecovery(property, true);
}

/**
 * @tc.name: SetCreateKeyboardSessionListener
 * @tc.desc: SetCreateKeyboardSessionListener
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, SetCreateKeyboardSessionListener, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    ssm_->SetCreateSystemSessionListener(nullptr);
    SessionInfo sessionInfo;
    sessionInfo.bundleName_ = "test1";
    sessionInfo.abilityName_ = "test2";
    sessionInfo.abilityInfo = nullptr;
    sessionInfo.isAtomicService_ = true;
    sessionInfo.screenId_ = SCREEN_ID_INVALID;
    ssm_->NotifySessionTouchOutside(123);
}

/**
 * @tc.name: DestroyAndDisconnectSpecificSessionInner
 * @tc.desc: check func DestroyAndDisconnectSpecificSessionInner
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, DestroyAndDisconnectSpecificSessionInner02, Function | SmallTest | Level2)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(nullptr, property);
    std::vector<int32_t> recoveredPersistentIds = {0, 1, 2};
    ssm_->SetAlivePersistentIds(recoveredPersistentIds);
    property->SetWindowType(WindowType::WINDOW_TYPE_DIALOG);
    ssm_->DestroyAndDisconnectSpecificSessionInner(1);
}

/**
 * @tc.name: DestroyToastSession
 * @tc.desc: DestroyToastSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, DestroyToastSession, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.screenId_ = SCREEN_ID_INVALID;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->DestroyToastSession(sceneSession);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->DestroyToastSession(sceneSession);
    ssm_->StartUIAbilityBySCB(sceneSession);
    int32_t ret = ssm_->ChangeUIAbilityVisibilityBySCB(sceneSession, true);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: RequestSceneSessionBackground
 * @tc.desc: RequestSceneSessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestSceneSessionBackground02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.ancoSceneState = 0;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<Session> session = new (std::nothrow) Session(info);
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
HWTEST_F(SceneSessionManagerTest5, ConfigAppWindowShadow03, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    WindowSceneConfig::ConfigItem item;
    WindowSceneConfig::ConfigItem shadowConfig;
    WindowShadowConfig outShadow;
    std::vector<float> floatTest = {0.0f, 0.1f, 0.2f, 0.3f};
    bool result = ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);
    ASSERT_EQ(result, true);

    item.SetValue(floatTest);
    shadowConfig.SetValue({{"offsetX", item}});
    ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);

    shadowConfig.SetValue({{"offsetY", item}});
    ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);

    item.SetValue(new std::string(""));
    shadowConfig.SetValue({{"color", item}});
    ssm_->ConfigAppWindowShadow(shadowConfig, outShadow);
}

/**
 * @tc.name: CreateAndConnectSpecificSession
 * @tc.desc: CreateAndConnectSpecificSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CreateAndConnectSpecificSession02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<ISessionStage> sessionStage = new (std::nothrow) SessionStageMocker();
    ASSERT_NE(sessionStage, nullptr);
    sptr<IWindowEventChannel> eventChannel = new (std::nothrow) WindowEventChannelMocker(sessionStage);
    ASSERT_NE(eventChannel, nullptr);
    std::shared_ptr<RSSurfaceNode> node = nullptr;
    sptr<ISession> session;
    SystemSessionConfig systemConfig;
    sptr<IRemoteObject> token;
    int32_t id = 0;
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    CommonTestUtils::GuaranteeFloatWindowPermission("ws_scene_session_manager_test5");
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property->SetWindowFlags(123);
    WSError res = ssm_->CreateAndConnectSpecificSession(sessionStage, eventChannel, node, property, id, session,
         systemConfig, token);
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, res); // create main window, property must be nullptr
    sessionStage = new (std::nothrow) SessionStageMocker();

    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    res = ssm_->CreateAndConnectSpecificSession(sessionStage, eventChannel, node, property, id, session,
        systemConfig, token);
    ASSERT_EQ(WSError::WS_OK, res);
    property->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    res = ssm_->CreateAndConnectSpecificSession(sessionStage, eventChannel, node, property, id, session,
        systemConfig, token);
    ASSERT_EQ(WSError::WS_OK, res);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    res = ssm_->CreateAndConnectSpecificSession(sessionStage, eventChannel, node, property, id, session,
        systemConfig, token);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: CheckSubSessionStartedByExtensionAndSetDisplayId
 * @tc.desc: CheckSubSessionStartedByExtensionAndSetDisplayId
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CheckUIExtensionAndSetDisplayId01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<SessionStageMocker> sessionStage = sptr<SessionStageMocker>::MakeSptr();
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sptr<IRemoteObject> token;
    SessionInfo info;
    sptr<SceneSession::SpecificSessionCallback> callback = ssm_->CreateSpecificSessionCallback();
    sptr<SceneSession> parentSession = sptr<SceneSession>::MakeSptr(info, callback);
    ssm_->sceneSessionMap_.insert({ parentSession->GetPersistentId(), parentSession });
    EXPECT_EQ(ssm_->CheckSubSessionStartedByExtensionAndSetDisplayId(token, property, sessionStage),
        WSError::WS_ERROR_NULLPTR);
    int64_t displayId = 1234;
    property->SetParentPersistentId(parentSession->GetPersistentId());
    parentSession->GetSessionProperty()->SetDisplayId(displayId);
    EXPECT_CALL(*sessionStage, UpdateDisplayId(displayId)).Times(1);
    EXPECT_EQ(ssm_->CheckSubSessionStartedByExtensionAndSetDisplayId(token, property, sessionStage),
        WSError::WS_OK);
    EXPECT_EQ(property->GetDisplayId(), displayId);
}

/**
 * @tc.name: ProcessDialogRequestFocusImmdediately
 * @tc.desc: ProcessDialogRequestFocusImmdediately
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, ProcessDialogRequestFocusImmdediately02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.persistentId_ = 123;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    auto ret = ssm_->ProcessDialogRequestFocusImmdediately(sceneSession);
    EXPECT_EQ(WSError::WS_DO_NOTHING, ret);
}

/**
 * @tc.name: RequestSceneSessionByCall
 * @tc.desc: SceneSesionManager request scene session by call
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestSceneSessionByCall02, Function | SmallTest | Level3)
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
HWTEST_F(SceneSessionManagerTest5, GetAllAbilityInfos02, Function | SmallTest | Level3)
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
 * @tc.name: GetBatchAbilityInfos01
 * @tc.desc: GetBatchAbilityInfos01
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, GetBatchAbilityInfos01, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    auto bundleMgr = ssm_->bundleMgr_;
    ssm_->bundleMgr_ = nullptr;
    int32_t userId = 100;
    std::vector<std::string> bundleNames = { "test1", "test2" };
    auto scbAbilityInfos = std::make_shared<std::vector<SCBAbilityInfo>>();
    WSError ret = ssm_->GetBatchAbilityInfos(bundleNames, userId, *scbAbilityInfos);
    ASSERT_EQ(ret, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: GetBatchAbilityInfos02
 * @tc.desc: GetBatchAbilityInfos02
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, GetBatchAbilityInfos02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ssm_->bundleMgr_ = sptr<AppExecFwk::BundleMgrProxy>::MakeSptr(iRemoteObjectMocker);
    int32_t userId = 100;
    std::vector<std::string> bundleNames = {};
    auto scbAbilityInfos = std::make_shared<std::vector<SCBAbilityInfo>>();
    WSError ret = ssm_->GetBatchAbilityInfos(bundleNames, userId, *scbAbilityInfos);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: GetBatchAbilityInfos03
 * @tc.desc: GetBatchAbilityInfos03
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, GetBatchAbilityInfos03, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ssm_->bundleMgr_ = sptr<AppExecFwk::BundleMgrProxy>::MakeSptr(iRemoteObjectMocker);
    int32_t userId = 100;
    std::vector<std::string> bundleNames = { "" };
    auto scbAbilityInfos = std::make_shared<std::vector<SCBAbilityInfo>>();
    WSError ret = ssm_->GetBatchAbilityInfos(bundleNames, userId, *scbAbilityInfos);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: GetBatchAbilityInfos04
 * @tc.desc: GetBatchAbilityInfos04
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, GetBatchAbilityInfos04, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    sptr<IRemoteObject> iRemoteObjectMocker = sptr<IRemoteObjectMocker>::MakeSptr();
    ssm_->bundleMgr_ = sptr<AppExecFwk::BundleMgrProxy>::MakeSptr(iRemoteObjectMocker);
    int32_t userId = 100;
    std::vector<std::string> bundleNames = { "test1", "test2" };
    auto scbAbilityInfos = std::make_shared<std::vector<SCBAbilityInfo>>();
    WSError ret = ssm_->GetBatchAbilityInfos(bundleNames, userId, *scbAbilityInfos);
    ASSERT_EQ(ret, WSError::WS_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: FindMainWindowWithToken
 * @tc.desc: SceneSesionManager find main window with token
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, FindMainWindowWithToken02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.persistentId_ = 123;
    sptr<IRemoteObject> targetToken = nullptr;
    ssm_->FindMainWindowWithToken(targetToken);
    targetToken = new (std::nothrow) IRemoteObjectMocker();
    ASSERT_NE(targetToken, nullptr);
    ssm_->FindMainWindowWithToken(targetToken);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ssm_->FindMainWindowWithToken(targetToken);
}

/**
 * @tc.name: RequestSceneSessionBackground
 * @tc.desc: RequestSceneSessionBackground
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestSceneSessionBackground03, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.persistentId_ = 0;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    sptr<Session> session = new (std::nothrow) Session(info);
    ASSERT_NE(session, nullptr);
    std::shared_ptr<std::promise<int32_t>> promise = std::make_shared<std::promise<int32_t>>();
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->SetSessionInfoPersistentId(0);
    ssm_->RequestSceneSessionBackground(sceneSession, false, false, true);
    
    ssm_->sceneSessionMap_.clear();
    ssm_->sceneSessionMap_.insert({0, sceneSession});
    ssm_->RequestSceneSessionBackground(sceneSession, false, false, true);
    ssm_->sceneSessionMap_.clear();
}

/**
 * @tc.name: DestroyToastSession
 * @tc.desc: DestroyToastSession
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, DestroyToastSession02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    info.screenId_ = SCREEN_ID_INVALID;
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    sptr<SceneSession> sceneSession = nullptr;
    ssm_->DestroyToastSession(sceneSession);
    sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    sceneSession->state_ = SessionState::STATE_FOREGROUND;
    ssm_->DestroyToastSession(sceneSession);
}

/**
 * @tc.name: CheckModalSubWindowPermission
 * @tc.desc: CheckModalSubWindowPermission
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, CheckModalSubWindowPermission02, Function | SmallTest | Level3)
{
    ASSERT_NE(ssm_, nullptr);
    SessionInfo info;
    info.abilityName_ = "test1";
    info.bundleName_ = "test2";
    sptr<WindowSessionProperty> property = new (std::nothrow) WindowSessionProperty();
    ASSERT_NE(property, nullptr);
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    property->SetWindowFlags(123);
    property->SetTopmost(true);
    ssm_->CheckModalSubWindowPermission(property);
    property->SetWindowType(WindowType::APP_SUB_WINDOW_BASE);
    ssm_->CheckModalSubWindowPermission(property);
}

/**
 * @tc.name: RequestFocusStatusBySCB
 * @tc.desc: SceneSessionManager request focus status from SCB
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionManagerTest5, RequestFocusStatusBySCB, Function | SmallTest | Level3)
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
    ssm_->sceneSessionMap_.insert({sceneSession->GetPersistentId(), sceneSession});

    FocusChangeReason reason = FocusChangeReason::FOREGROUND;
    ssm_->RequestFocusStatusBySCB(1, true, false, reason);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(ssm_->focusedSessionId_, 1);

    reason = FocusChangeReason::CLICK;
    ssm_->RequestFocusStatusBySCB(1, true, false, reason);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(ssm_->focusedSessionId_, 1);

    ssm_->RequestFocusStatusBySCB(1, false, false, reason);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_NE(ssm_->focusedSessionId_, 1);

    reason = FocusChangeReason::MOVE_UP;
    ssm_->RequestFocusStatusBySCB(1, true, false, reason);
    usleep(WAIT_SYNC_IN_NS);
    FocusChangeReason reasonResult = sceneSession->GetPostProcessFocusState().reason_;
    ASSERT_EQ(reasonResult, FocusChangeReason::MOVE_UP);

    reason = FocusChangeReason::DEFAULT;
    ssm_->RequestFocusStatusBySCB(1, true, false, reason);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(ssm_->focusedSessionId_, 1);
}

}
} // namespace Rosen
} // namespace OHOS
