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

#include "common/include/window_session_property.h"
#include <gtest/gtest.h>
#include "input_manager.h"
#include <parameter.h>
#include <parameters.h>
#include "session_manager/include/scene_session_dirty_manager.h"
#include "screen_session_manager_client/include/screen_session_manager_client.h"
#include "scene_input_manager.h"
#include "session/host/include/move_drag_controller.h"
#include "session/host/include/scene_session.h"
#include "session_manager/include/scene_session_manager.h"
#include "transaction/rs_uiextension_data.h"
#include "window_helper.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

class SceneSessionDirtyManagerTest2 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    static constexpr uint32_t WAIT_SYNC_IN_NS = 500000;
};

std::shared_ptr<SceneSessionDirtyManager> manager_;
sptr<SceneSessionManager> ssm_;
std::shared_ptr<SceneInputManager> sim_;

void SceneSessionDirtyManagerTest2::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
    ssm_->sceneSessionMap_.clear();
    sim_ = std::make_shared<SceneInputManager>();
}

void SceneSessionDirtyManagerTest2::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SceneSessionDirtyManagerTest2::SetUp()
{
    manager_ = std::make_shared<SceneSessionDirtyManager>();
}

void SceneSessionDirtyManagerTest2::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
    manager_ = nullptr;
}

namespace {
void InitSessionInfo(MMI::DisplayInfo& displayedInfo, MMI::WindowInfo& windowInfo)
{
    displayedInfo = { .id = 42, .x = 0, .y = 0, .width = 1270, .height = 2240 };

    windowInfo = { .id = 43, .pid = 433, .displayId = 42, .zOrder = 30.0, .area = { 0, 0, 1000, 1200 } };
}

/**
 * @tc.name: Init sceneSession
 * @tc.desc: Init sceneSession
 * @tc.type: FUNC
 */
void InitSceneSession(sptr<SceneSession>& sceneSession, int32_t pid, int windowId, WindowType propertyType)
{
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(windowSessionProperty, nullptr);
    uint64_t displayId = 1;
    windowSessionProperty->SetDisplayId(displayId);
    windowSessionProperty->SetWindowMode(WindowMode::WINDOW_MODE_UNDEFINED);
    windowSessionProperty->SetMaximizeMode(MaximizeMode::MODE_AVOID_SYSTEM_BAR);
    windowSessionProperty->SetWindowType(propertyType);
    windowSessionProperty->topmost_ = false;
    windowSessionProperty->SetPersistentId(windowId);
    sceneSession->SetSessionProperty(windowSessionProperty);

    WSRect windowRect = { 0, 0, 1270, 2700 };
    sceneSession->SetSessionRect(windowRect);
    sceneSession->SetVisibilityChangedDetectFunc([](int32_t pid, bool isVisible, bool newIsVisible) { return; });
    sceneSession->SetCallingPid(pid);
    int32_t uid = 1315;
    sceneSession->SetCallingUid(uid);
}

/**
 * @tc.name: StartDelayedFlushWindowInfoToMMITask
 * @tc.desc: StartDelayedFlushWindowInfoToMMITask
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, StartDelayedFlushWindowInfoToMMITask, TestSize.Level1)
{
    ssm_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ssm_->isDelayFlushWindowInfoMode_ = true;
    ssm_->HandleUserSwitching(false);
    EXPECT_EQ(ssm_->isDelayFlushWindowInfoMode_, false);

    ssm_->isDelayFlushWindowInfoMode_ = true;
    ssm_->HandleUserSwitching(true);
    EXPECT_EQ(ssm_->isDelayFlushWindowInfoMode_, true);
    sleep(2);

    ssm_->isUserBackground_ = true;
    ssm_->isDelayFlushWindowInfoMode_ = false;
    ssm_->HandleUserSwitching(true);
    EXPECT_EQ(ssm_->isDelayFlushWindowInfoMode_, true);
    sleep(2);
    
    ssm_->isUserBackground_ = false;
    ssm_->isDelayFlushWindowInfoMode_ = false;
    ssm_->HandleUserSwitching(true);
    EXPECT_EQ(ssm_->isDelayFlushWindowInfoMode_, true);
    sleep(2);
}

/**
 * @tc.name: GetWindowInfoWithoutParentWindowAndStateActive
 * @tc.desc: windowInfo without parent window and state active
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithoutParentWindowAndStateActive, TestSize.Level1)
{
    SessionInfo subWindowInfo;
    subWindowInfo.abilityName_ = "TestSubWithType";
    subWindowInfo.bundleName_ = "TestSubWithType";
    int32_t subWindowId = 36;
    sptr<SceneSession> sceneSessionSubWindow = sptr<SceneSession>::MakeSptr(subWindowInfo, nullptr);
    ASSERT_NE(sceneSessionSubWindow, nullptr);
    sceneSessionSubWindow->persistentId_ = subWindowId;
    sceneSessionSubWindow->isVisible_ = false;
    uint32_t subFlags = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK);
    sceneSessionSubWindow->property_->SetWindowFlags(subFlags);
    int32_t subWindowPid = 56;
    InitSceneSession(sceneSessionSubWindow, subWindowPid, subWindowId, WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSessionSubWindow->SetParentSession(nullptr);
    sceneSessionSubWindow->SetSessionState(SessionState::STATE_ACTIVE);
    std::map<int32_t, sptr<SceneSession>> retSceneSessionMap;
    retSceneSessionMap.insert(std::make_pair(subWindowPid, sceneSessionSubWindow));
    ssm_->sceneSessionMap_ = retSceneSessionMap;
    std::map<int32_t, sptr<SceneSession>> sceneSessionMap =
        Rosen::SceneSessionManager::GetInstance().GetSceneSessionMap();
    int32_t windowInfoSize = sceneSessionMap.size();
    ASSERT_EQ(windowInfoSize, 0);
}

/**
 * @tc.name: GetWindowInfoWithNotSystemTouchable
 * @tc.desc: windowInfo with systemtouchable is false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithNotSystemTouchable, TestSize.Level1)
{
    SessionInfo mainWindowInfo;
    mainWindowInfo.abilityName_ = "TestMainWithType";
    mainWindowInfo.bundleName_ = "TestMainWithType";
    int32_t mainWindowId = 37;
    sptr<SceneSession> sceneSessionMainWindow = sptr<SceneSession>::MakeSptr(mainWindowInfo, nullptr);
    ASSERT_NE(sceneSessionMainWindow, nullptr);
    sceneSessionMainWindow->persistentId_ = mainWindowId;
    sceneSessionMainWindow->isVisible_ = true;
    uint32_t mainFlags = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK);
    sceneSessionMainWindow->property_->SetWindowFlags(mainFlags);
    bool isTouchable = false;
    // set systemTouchable false;
    sceneSessionMainWindow->SetSystemTouchable(isTouchable);
    int32_t mainWindowPid = 57;
    InitSceneSession(sceneSessionMainWindow, mainWindowPid, mainWindowId, WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    std::map<int32_t, sptr<SceneSession>> retSceneSessionMap;
    retSceneSessionMap.insert(std::make_pair(mainWindowPid, sceneSessionMainWindow));
    ssm_->sceneSessionMap_ = retSceneSessionMap;
    MMI::WindowInfo windowInfo;
    ScreenId screenId = 37;
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    manager_->UpdateWindowFlags(screenId, sceneSessionMainWindow, windowInfo);
    ASSERT_EQ(windowInfo.flags, 1);
}

/**
 * @tc.name: GetWindowInfoWithNotIsTouchEnable
 * @tc.desc: windowInfo with isTouchenable is false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithNotIsTouchEnable, TestSize.Level1)
{
    SessionInfo mainWindowInfo;
    mainWindowInfo.abilityName_ = "TestMainWithType";
    mainWindowInfo.bundleName_ = "TestMainWithType";
    int32_t mainWindowId = 38;
    sptr<SceneSession> sceneSessionMainWindow = sptr<SceneSession>::MakeSptr(mainWindowInfo, nullptr);
    ASSERT_NE(sceneSessionMainWindow, nullptr);
    sceneSessionMainWindow->persistentId_ = mainWindowId;
    sceneSessionMainWindow->isVisible_ = true;
    uint32_t mainFlags = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK);
    sceneSessionMainWindow->property_->SetWindowFlags(mainFlags);
    bool isTouchable = true;
    sceneSessionMainWindow->SetSystemTouchable(isTouchable);
    int32_t mainWindowPid = 58;
    InitSceneSession(sceneSessionMainWindow, mainWindowPid, mainWindowId, WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    std::map<int32_t, sptr<SceneSession>> retSceneSessionMap;
    retSceneSessionMap.insert(std::make_pair(mainWindowPid, sceneSessionMainWindow));
    ssm_->sceneSessionMap_ = retSceneSessionMap;
    MMI::WindowInfo windowInfo;
    ScreenId screenId = 38;
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    // set screenSession touchenable_ true
    screenSession->touchEnabled_.store(false);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    manager_->UpdateWindowFlags(screenId, sceneSessionMainWindow, windowInfo);
    ASSERT_EQ(windowInfo.flags, MMI::WindowInfo::FLAG_BIT_UNTOUCHABLE);
}

/**
 * @tc.name: GetWindowInfoWithNotIsForceTouchEnable
 * @tc.desc: windowInfo with isForeTouchenable is false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithNotIsForceTouchEnable, TestSize.Level1)
{
    SessionInfo mainWindowInfo;
    mainWindowInfo.abilityName_ = "TestMainWithType";
    mainWindowInfo.bundleName_ = "TestMainWithType";
    int32_t mainWindowId = 39;
    sptr<SceneSession> sceneSessionMainWindow = sptr<SceneSession>::MakeSptr(mainWindowInfo, nullptr);
    ASSERT_NE(sceneSessionMainWindow, nullptr);
    sceneSessionMainWindow->persistentId_ = mainWindowId;
    sceneSessionMainWindow->isVisible_ = true;
    uint32_t mainFlags = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK);
    sceneSessionMainWindow->property_->SetWindowFlags(mainFlags);
    // set isForceTouchable false
    bool isForceTouchable = false;
    sceneSessionMainWindow->SetForceTouchable(isForceTouchable);
    int32_t mainWindowPid = 59;
    InitSceneSession(sceneSessionMainWindow, mainWindowPid, mainWindowId, WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    std::map<int32_t, sptr<SceneSession>> retSceneSessionMap;
    retSceneSessionMap.insert(std::make_pair(mainWindowPid, sceneSessionMainWindow));
    ssm_->sceneSessionMap_ = retSceneSessionMap;
    MMI::WindowInfo windowInfo;
    ScreenId screenId = 39;
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    screenSession->touchEnabled_.store(true);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    manager_->UpdateWindowFlags(screenId, sceneSessionMainWindow, windowInfo);
    ASSERT_EQ(windowInfo.flags, 1);
}

/**
 * @tc.name: GetWindowInfoWithNotForegroundInteractiveStatus
 * @tc.desc: windowInfo with foregroundInteractiveStatus_ is false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithNotForegroundInteractiveStatus, TestSize.Level1)
{
    SessionInfo mainWindowInfo;
    mainWindowInfo.abilityName_ = "TestMainWithType";
    mainWindowInfo.bundleName_ = "TestMainWithType";
    int32_t mainWindowId = 40;
    sptr<SceneSession> sceneSessionMainWindow = sptr<SceneSession>::MakeSptr(mainWindowInfo, nullptr);
    ASSERT_NE(sceneSessionMainWindow, nullptr);
    sceneSessionMainWindow->persistentId_ = mainWindowId;
    sceneSessionMainWindow->isVisible_ = true;
    // set foregroundInterativeStatus_ false
    sceneSessionMainWindow->foregroundInteractiveStatus_.store(false);
    uint32_t mainFlags = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK);
    sceneSessionMainWindow->property_->SetWindowFlags(mainFlags);
    int32_t mainWindowPid = 60;
    InitSceneSession(sceneSessionMainWindow, mainWindowPid, mainWindowId, WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    std::map<int32_t, sptr<SceneSession>> retSceneSessionMap;
    retSceneSessionMap.insert(std::make_pair(mainWindowPid, sceneSessionMainWindow));
    ssm_->sceneSessionMap_ = retSceneSessionMap;
    MMI::WindowInfo windowInfo;
    ScreenId screenId = 40;
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    manager_->UpdateWindowFlags(screenId, sceneSessionMainWindow, windowInfo);
    ASSERT_EQ(windowInfo.flags, 1);
}

/**
 * @tc.name: GetWindowInfoWithNotPropertyTouchable
 * @tc.desc: windowInfo with property touchable false
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithNotPropertyTouchable, TestSize.Level1)
{
    SessionInfo mainWindowInfo;
    mainWindowInfo.abilityName_ = "TestMainWithType";
    mainWindowInfo.bundleName_ = "TestMainWithType";
    int32_t mainWindowId = 41;
    sptr<SceneSession> sceneSessionMainWindow = sptr<SceneSession>::MakeSptr(mainWindowInfo, nullptr);
    ASSERT_NE(sceneSessionMainWindow, nullptr);
    sceneSessionMainWindow->persistentId_ = mainWindowId;
    sceneSessionMainWindow->foregroundInteractiveStatus_.store(false);
    uint32_t mainFlags = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK);
    sceneSessionMainWindow->property_->SetWindowFlags(mainFlags);
    // set property_.touchEnable_ false
    sceneSessionMainWindow->property_->touchable_ = false;
    int32_t mainWindowPid = 61;
    InitSceneSession(sceneSessionMainWindow, mainWindowPid, mainWindowId, WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    std::map<int32_t, sptr<SceneSession>> retSceneSessionMap;
    retSceneSessionMap.insert(std::make_pair(mainWindowPid, sceneSessionMainWindow));
    ssm_->sceneSessionMap_ = retSceneSessionMap;
    MMI::WindowInfo windowInfo;
    ScreenId screenId = 41;
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    manager_->UpdateWindowFlags(screenId, sceneSessionMainWindow, windowInfo);
    ASSERT_EQ(windowInfo.flags, 1);
}

/**
 * @tc.name: GetWindowInfoWithSameInfo
 * @tc.desc: windowInfo with same displayInfos and windowInfos
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithSameInfo, TestSize.Level1)
{
    MMI::DisplayInfo lastDisplayedInfo;
    MMI::WindowInfo lastWindowInfo;
    InitSessionInfo(lastDisplayedInfo, lastWindowInfo);
    int32_t focusedSession = 42;
    sim_->focusedSessionId_ = focusedSession;
    int32_t lastFocusedSession = 42;
    sim_->lastFocusId_ = lastFocusedSession;
    std::vector<MMI::DisplayInfo> lastDisplayInfos;
    std::vector<MMI::WindowInfo> lastWindowInfoList;
    lastDisplayInfos.emplace_back(lastDisplayedInfo);
    lastWindowInfoList.emplace_back(lastWindowInfo);
    sim_->lastDisplayInfos_ = lastDisplayInfos;
    sim_->lastWindowInfoList_ = lastWindowInfoList;
    MMI::DisplayInfo currDisplayedInfo;
    MMI::WindowInfo currWindowInfo;
    InitSessionInfo(currDisplayedInfo, currWindowInfo);
    std::vector<MMI::DisplayInfo> currDisplayInfos;
    std::vector<MMI::WindowInfo> currWindowInfoList;
    currDisplayInfos.emplace_back(currDisplayedInfo);
    currWindowInfoList.emplace_back(currWindowInfo);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(focusedSession);
    bool checkNeedUpdateFlag = true;
    std::vector<MMI::ScreenInfo> screenInfos;
    // check the same information
    checkNeedUpdateFlag = sim_->CheckNeedUpdate(screenInfos, currDisplayInfos, currWindowInfoList);
    ASSERT_EQ(checkNeedUpdateFlag, false);
}

/**
 * @tc.name: GetWindowInfoWithPidDiff
 * @tc.desc: windowInfo with different pid
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithPidDiff, TestSize.Level1)
{
    MMI::DisplayInfo lastDisplayedInfo;
    MMI::WindowInfo lastWindowInfo;
    InitSessionInfo(lastDisplayedInfo, lastWindowInfo);
    int32_t focusedSession = 43;
    sim_->focusedSessionId_ = focusedSession;
    int32_t lastFocusedSession = 43;
    sim_->lastFocusId_ = lastFocusedSession;
    std::vector<MMI::DisplayInfo> lastDisplayInfos;
    std::vector<MMI::WindowInfo> lastWindowInfoList;
    lastDisplayInfos.emplace_back(lastDisplayedInfo);
    lastWindowInfoList.emplace_back(lastWindowInfo);
    sim_->lastDisplayInfos_ = lastDisplayInfos;
    sim_->lastWindowInfoList_ = lastWindowInfoList;
    MMI::DisplayInfo currDisplayedInfo;
    MMI::WindowInfo currWindowInfo;
    InitSessionInfo(currDisplayedInfo, currWindowInfo);
    // set different pid number
    currWindowInfo.pid = 434;
    std::vector<MMI::DisplayInfo> currDisplayInfos;
    std::vector<MMI::WindowInfo> currWindowInfoList;
    currDisplayInfos.emplace_back(currDisplayedInfo);
    currWindowInfoList.emplace_back(currWindowInfo);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(focusedSession);
    bool checkNeedUpdateFlag = false;
    std::vector<MMI::ScreenInfo> screenInfos;
    checkNeedUpdateFlag = sim_->CheckNeedUpdate(screenInfos, currDisplayInfos, currWindowInfoList);
    ASSERT_EQ(checkNeedUpdateFlag, true);
}

/**
 * @tc.name: GetWindowInfoWithAreaDiff
 * @tc.desc: windowInfo with different area
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithAreaDiff, TestSize.Level1)
{
    MMI::DisplayInfo lastDisplayedInfo;
    MMI::WindowInfo lastWindowInfo;
    InitSessionInfo(lastDisplayedInfo, lastWindowInfo);
    int32_t focusedSession = 44;
    sim_->focusedSessionId_ = focusedSession;
    int32_t lastFocusedSession = 44;
    sim_->lastFocusId_ = lastFocusedSession;
    std::vector<MMI::DisplayInfo> lastDisplayInfos;
    std::vector<MMI::WindowInfo> lastWindowInfoList;
    lastDisplayInfos.emplace_back(lastDisplayedInfo);
    lastWindowInfoList.emplace_back(lastWindowInfo);
    sim_->lastDisplayInfos_ = lastDisplayInfos;
    sim_->lastWindowInfoList_ = lastWindowInfoList;
    MMI::DisplayInfo currDisplayedInfo;
    MMI::WindowInfo currWindowInfo;
    InitSessionInfo(currDisplayedInfo, currWindowInfo);
    // set different area number
    currWindowInfo.area = { 0, 0, 500, 600 };
    std::vector<MMI::DisplayInfo> currDisplayInfos;
    std::vector<MMI::WindowInfo> currWindowInfoList;
    currDisplayInfos.emplace_back(currDisplayedInfo);
    currWindowInfoList.emplace_back(currWindowInfo);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(focusedSession);
    bool checkNeedUpdateFlag = false;
    std::vector<MMI::ScreenInfo> screenInfos;
    checkNeedUpdateFlag = sim_->CheckNeedUpdate(screenInfos, currDisplayInfos, currWindowInfoList);
    ASSERT_EQ(checkNeedUpdateFlag, true);
}

/**
 * @tc.name: GetWindowInfoWithzOrderDiff
 * @tc.desc: windowInfo with different zOrder
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithzOrderDiff, TestSize.Level1)
{
    MMI::DisplayInfo lastDisplayedInfo;
    MMI::WindowInfo lastWindowInfo;
    InitSessionInfo(lastDisplayedInfo, lastWindowInfo);
    int32_t focusedSession = 45;
    sim_->focusedSessionId_ = focusedSession;
    int32_t lastFocusedSession = 45;
    sim_->lastFocusId_ = lastFocusedSession;
    std::vector<MMI::DisplayInfo> lastDisplayInfos;
    std::vector<MMI::WindowInfo> lastWindowInfoList;
    lastDisplayInfos.emplace_back(lastDisplayedInfo);
    lastWindowInfoList.emplace_back(lastWindowInfo);
    sim_->lastDisplayInfos_ = lastDisplayInfos;
    sim_->lastWindowInfoList_ = lastWindowInfoList;
    MMI::DisplayInfo currDisplayedInfo;
    MMI::WindowInfo currWindowInfo;
    InitSessionInfo(currDisplayedInfo, currWindowInfo);
    // set different zOrder number
    currWindowInfo.zOrder = 40.0;
    std::vector<MMI::DisplayInfo> currDisplayInfos;
    std::vector<MMI::WindowInfo> currWindowInfoList;
    currDisplayInfos.emplace_back(currDisplayedInfo);
    currWindowInfoList.emplace_back(currWindowInfo);
    auto focusGroup = ssm_->windowFocusController_->GetFocusGroup(DEFAULT_DISPLAY_ID);
    focusGroup->SetFocusedSessionId(focusedSession);
    bool checkNeedUpdateFlag = false;
    std::vector<MMI::ScreenInfo> screenInfos;
    checkNeedUpdateFlag = sim_->CheckNeedUpdate(screenInfos, currDisplayInfos, currWindowInfoList);
    ASSERT_EQ(checkNeedUpdateFlag, true);
}

/**
 * @tc.name: GetWindowInfoWithoutHotArea
 * @tc.desc: windowInfo without hotAreas information
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithoutHotArea, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "TestWithoutHotArea";
    info.bundleName_ = "TestWithoutHotArea";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(windowSessionProperty, nullptr);
    windowSessionProperty->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    sceneSession->InitSessionPropertyWhenConnect(windowSessionProperty);
    WSRect windowRect = { 0, 0, 1270, 2700 };
    sceneSession->SetSessionRect(windowRect);
    sceneSession->SetSessionGlobalRect(windowRect);
    // set hotArea without info
    std::vector<MMI::Rect> touchHotAreas;
    std::vector<MMI::Rect> pointerHotAreas;
    manager_->UpdateHotAreas(sceneSession, touchHotAreas, pointerHotAreas);
    bool touchHotResult = touchHotAreas[0].x == 0 && touchHotAreas[0].y == 0 && touchHotAreas[0].width == 1270 &&
                          touchHotAreas[0].height == 2700;
    ASSERT_EQ(touchHotResult, true);
    bool pointerHotResult = pointerHotAreas[0].x == 0 && pointerHotAreas[0].y == 0 &&
                            pointerHotAreas[0].width == 1270 && pointerHotAreas[0].height == 2700;
    ASSERT_EQ(pointerHotResult, true);
}

/**
 * @tc.name: GetWindowInfoWithHotArea
 * @tc.desc: windowInfo with hotAreas information
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithHotArea, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "TestWithHotArea";
    info.bundleName_ = "TestWithHotArea";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(sceneSession, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = sptr<WindowSessionProperty>::MakeSptr();
    ASSERT_NE(windowSessionProperty, nullptr);
    windowSessionProperty->SetWindowType(WindowType::WINDOW_TYPE_GLOBAL_SEARCH);
    Rect rect = { 0, 0, 300, 500 };
    std::vector<Rect> rects;
    rects.emplace_back(rect);
    // set touchHotArea and pointerHotArea info
    windowSessionProperty->SetTouchHotAreas(rects);
    sceneSession->InitSessionPropertyWhenConnect(windowSessionProperty);
    WSRect windowRect = { 0, 0, 1270, 2700 };
    sceneSession->SetSessionRect(windowRect);
    std::vector<MMI::Rect> touchHotAreas;
    std::vector<MMI::Rect> pointerHotAreas;
    manager_->UpdateHotAreas(sceneSession, touchHotAreas, pointerHotAreas);
    bool touchHotResult = touchHotAreas[0].x == 0 && touchHotAreas[0].y == 0 && touchHotAreas[0].width == 300 &&
                          touchHotAreas[0].height == 500;
    ASSERT_EQ(touchHotResult, true);
    bool pointerHotResult = pointerHotAreas[0].x == 0 && pointerHotAreas[0].y == 0 && pointerHotAreas[0].width == 300 &&
                            pointerHotAreas[0].height == 500;
    ASSERT_EQ(pointerHotResult, true);
}

/**
 * @tc.name: GetWindowInfoWithWindowTypeDialog
 * @tc.desc: windowInfo with windowType dialog
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithWindowTypeDialog, TestSize.Level1)
{
    // init main window info
    SessionInfo mainWindowInfo;
    mainWindowInfo.abilityName_ = "TestMainWithType";
    mainWindowInfo.bundleName_ = "TestMainWithType";
    int32_t mainWindowId = 30;
    sptr<SceneSession> sceneSessionMainWindow = sptr<SceneSession>::MakeSptr(mainWindowInfo, nullptr);
    ASSERT_NE(sceneSessionMainWindow, nullptr);
    sceneSessionMainWindow->zOrder_ = 55.0f;
    sceneSessionMainWindow->persistentId_ = mainWindowId;
    sceneSessionMainWindow->isVisible_ = true;
    int32_t mainFlags = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK);
    sceneSessionMainWindow->property_->SetWindowFlags(mainFlags);
    int32_t mainWindowPid = 50;
    InitSceneSession(sceneSessionMainWindow, mainWindowPid, mainWindowId, WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    // init sub window with window type dialog
    SessionInfo dialogWindowInfo;
    dialogWindowInfo.abilityName_ = "TestDialogWithType";
    dialogWindowInfo.bundleName_ = "TestDialogWithType";
    int32_t dialogWindowId = 31;
    sptr<SceneSession> sceneSessionDialogWindow = sptr<SceneSession>::MakeSptr(dialogWindowInfo, nullptr);
    ASSERT_NE(sceneSessionDialogWindow, nullptr);
    sceneSessionDialogWindow->zOrder_ = 56.0f;
    sceneSessionDialogWindow->persistentId_ = dialogWindowId;
    sceneSessionDialogWindow->isVisible_ = true;
    uint32_t flags = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_MODAL);
    sceneSessionDialogWindow->property_->SetWindowFlags(flags);
    int32_t dialogWindowPid = 51;
    InitSceneSession(sceneSessionDialogWindow, dialogWindowPid, dialogWindowId, WindowType::WINDOW_TYPE_DIALOG);
    sceneSessionDialogWindow->SetParentSession(sceneSessionMainWindow);
    std::map<int32_t, sptr<SceneSession>> retSceneSessionMap;
    retSceneSessionMap.insert(std::make_pair(mainWindowPid, sceneSessionMainWindow));
    retSceneSessionMap.insert(std::make_pair(dialogWindowPid, sceneSessionDialogWindow));
    ssm_->sceneSessionMap_ = retSceneSessionMap;
    auto [windowInfoList, pixelMapList] = manager_->GetFullWindowInfoList();
    ASSERT_EQ(windowInfoList.size(), 2);
    bool windowTypeDialogResult = false;
    for (MMI::WindowInfo windowInfo : windowInfoList) {
        if (windowInfo.id == mainWindowId && windowInfo.agentWindowId == dialogWindowId &&
            windowInfo.pid == dialogWindowPid) {
            windowTypeDialogResult = true;
        }
    }
    ASSERT_EQ(windowTypeDialogResult, true);
}

/**
 * @tc.name: GetWindowInfoWithWindowTypeAppSub
 * @tc.desc: windowInfo with window_Type_app_sub
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithWindowTypeAppSub, TestSize.Level1)
{
    // init main window info
    SessionInfo mainWindowInfo;
    mainWindowInfo.abilityName_ = "TestMainWithType";
    mainWindowInfo.bundleName_ = "TestMainWithType";
    int32_t mainWindowId = 32;
    sptr<SceneSession> sceneSessionMainWindow = sptr<SceneSession>::MakeSptr(mainWindowInfo, nullptr);
    ASSERT_NE(sceneSessionMainWindow, nullptr);
    sceneSessionMainWindow->persistentId_ = mainWindowId;
    sceneSessionMainWindow->isVisible_ = true;
    int32_t mainFlags = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK);
    sceneSessionMainWindow->property_->SetWindowFlags(mainFlags);
    int32_t mainWindowPid = 52;
    InitSceneSession(sceneSessionMainWindow, mainWindowPid, mainWindowId, WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    // init sub window with window type float
    SessionInfo subWindowInfo;
    subWindowInfo.abilityName_ = "TestSubWithType";
    subWindowInfo.bundleName_ = "TestSubWithType";
    int32_t subWindowId = 33;
    sptr<SceneSession> sceneSessionSubWindow = sptr<SceneSession>::MakeSptr(subWindowInfo, nullptr);
    ASSERT_NE(sceneSessionSubWindow, nullptr);
    sceneSessionSubWindow->persistentId_ = subWindowId;
    sceneSessionSubWindow->isVisible_ = true;
    uint32_t flags = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_IS_MODAL);
    sceneSessionSubWindow->property_->SetWindowFlags(flags);
    int32_t subWindowPid = 53;
    InitSceneSession(sceneSessionSubWindow, subWindowPid, subWindowId, WindowType::WINDOW_TYPE_FLOAT);
    sceneSessionSubWindow->SetParentSession(sceneSessionMainWindow);
    std::map<int32_t, sptr<SceneSession>> retSceneSessionMap;
    retSceneSessionMap.insert(std::make_pair(mainWindowPid, sceneSessionMainWindow));
    retSceneSessionMap.insert(std::make_pair(subWindowPid, sceneSessionSubWindow));
    ssm_->sceneSessionMap_ = retSceneSessionMap;
    auto [windowInfoList, pixelMapList] = manager_->GetFullWindowInfoList();
    ASSERT_EQ(windowInfoList.size(), 2);
    bool windowTypeDialogResult = false;
    for (MMI::WindowInfo windowInfo : windowInfoList) {
        if (windowInfo.id == mainWindowId && windowInfo.agentWindowId == subWindowId &&
            windowInfo.pid == subWindowPid) {
            windowTypeDialogResult = true;
        }
    }
    ASSERT_EQ(windowTypeDialogResult, false);
}

/**
 * @tc.name: GetWindowInfoWithTypeKeyboardPanel
 * @tc.desc: windowInfo with window_Type_keyboard panel
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithTypeKeyboardPanel, TestSize.Level1)
{
    SessionInfo mainWindowInfo;
    mainWindowInfo.abilityName_ = "TestMainWithType";
    mainWindowInfo.bundleName_ = "TestMainWithType";
    int32_t mainWindowId = 34;
    sptr<SceneSession> sceneSessionMainWindow = sptr<SceneSession>::MakeSptr(mainWindowInfo, nullptr);
    ASSERT_NE(sceneSessionMainWindow, nullptr);
    sceneSessionMainWindow->persistentId_ = mainWindowId;
    sceneSessionMainWindow->isVisible_ = false;
    uint32_t mainFlags = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK);
    sceneSessionMainWindow->property_->SetWindowFlags(mainFlags);
    int32_t mainWindowPid = 54;
    // init sceneSession with windowType is window_type_keyBoard_panel
    InitSceneSession(sceneSessionMainWindow, mainWindowPid, mainWindowId, WindowType::WINDOW_TYPE_KEYBOARD_PANEL);
    std::map<int32_t, sptr<SceneSession>> retSceneSessionMap;
    retSceneSessionMap.insert(std::make_pair(mainWindowPid, sceneSessionMainWindow));
    ssm_->sceneSessionMap_ = retSceneSessionMap;
    std::map<int32_t, sptr<SceneSession>> sceneSessionMap =
        Rosen::SceneSessionManager::GetInstance().GetSceneSessionMap();
    int32_t windowInfoSize = sceneSessionMap.size();
    ASSERT_EQ(windowInfoSize, 0);
}

/**
 * @tc.name: GetWindowInfoWithoutParentWindow
 * @tc.desc: windowInfo without parent window
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithoutParentWindow, TestSize.Level1)
{
    SessionInfo subWindowInfo;
    subWindowInfo.abilityName_ = "TestSubWithType";
    subWindowInfo.bundleName_ = "TestSubWithType";
    int32_t subWindowId = 35;
    sptr<SceneSession> sceneSessionSubWindow = sptr<SceneSession>::MakeSptr(subWindowInfo, nullptr);
    ASSERT_NE(sceneSessionSubWindow, nullptr);
    sceneSessionSubWindow->persistentId_ = subWindowId;
    sceneSessionSubWindow->isVisible_ = false;
    uint32_t subFlags = static_cast<uint32_t>(WindowFlag::WINDOW_FLAG_WATER_MARK);
    sceneSessionSubWindow->property_->SetWindowFlags(subFlags);
    int32_t subWindowPid = 55;
    InitSceneSession(sceneSessionSubWindow, subWindowPid, subWindowId, WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    sceneSessionSubWindow->SetParentSession(nullptr);
    std::map<int32_t, sptr<SceneSession>> retSceneSessionMap;
    retSceneSessionMap.insert(std::make_pair(subWindowPid, sceneSessionSubWindow));
    ssm_->sceneSessionMap_ = retSceneSessionMap;
    std::map<int32_t, sptr<SceneSession>> sceneSessionMap =
        Rosen::SceneSessionManager::GetInstance().GetSceneSessionMap();
    int32_t windowInfoSize = sceneSessionMap.size();
    ASSERT_EQ(windowInfoSize, 0);
}

/**
 * @tc.name: GetWindowInfoWithScreenshotPrefix
 * @tc.desc: windowInfo with screenshot prefix
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithScreenshotPrefix, TestSize.Level2)
{
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = session->GetSessionProperty();
    session->SetSessionProperty(windowSessionProperty);
    windowSessionProperty->SetWindowName("ScreenShotWindowExample");

    auto result = manager_->GetWindowInfo(session, SceneSessionDirtyManager::WindowAction::WINDOW_ADD);

    EXPECT_EQ(result.first.windowNameType, 1);
}

/**
 * @tc.name: GetWindowInfoWithPreviewPrefix
 * @tc.desc: windowInfo with preview prefix
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithPreviewPrefix, TestSize.Level2)
{
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = session->GetSessionProperty();
    session->SetSessionProperty(windowSessionProperty);
    windowSessionProperty->SetWindowName("PreviewWindowExample");

    auto result = manager_->GetWindowInfo(session, SceneSessionDirtyManager::WindowAction::WINDOW_ADD);

    EXPECT_EQ(result.first.windowNameType, 1);
}

/**
 * @tc.name: GetWindowInfoWithoutSpecialPrefix
 * @tc.desc: windowInfo without special prefix
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithoutSpecialPrefix, TestSize.Level2)
{
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = session->GetSessionProperty();
    session->SetSessionProperty(windowSessionProperty);
    windowSessionProperty->SetWindowName("RegularWindowExample");

    auto result = manager_->GetWindowInfo(session, SceneSessionDirtyManager::WindowAction::WINDOW_ADD);

    EXPECT_EQ(result.first.windowNameType, 0);
}

/**
 * @tc.name: GetWindowInfoWithVoiceInputPrefix
 * @tc.desc: windowInfo with preview prefix
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, GetWindowInfoWithVoiceInputPrefix, TestSize.Level2)
{
    std::vector<MMI::WindowInfo> lastWindowInfoList;
    SessionInfo info;
    sptr<SceneSession> session = new (std::nothrow) SceneSession(info, nullptr);
    sptr<WindowSessionProperty> windowSessionProperty = session->GetSessionProperty();
    session->SetSessionProperty(windowSessionProperty);
    windowSessionProperty->SetWindowName("__VoiceHardwareInputExample");
 
    auto result = manager_->GetWindowInfo(session, SceneSessionDirtyManager::WindowAction::WINDOW_ADD);
 
    EXPECT_EQ(result.first.windowNameType, 2);
}

/**
 * @tc.name: UpdateWindowFlagsForLockCursor
 * @tc.desc: UpdateWindowFlagsForLockCursor
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, UpdateWindowFlagsForLockCursor, TestSize.Level2)
{
    MMI::WindowInfo windowInfo;
    manager_->UpdateWindowFlagsForLockCursor(nullptr, windowInfo);
    EXPECT_EQ(windowInfo.flags, 0);

    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->persistentId_ = 5;
    session->UpdateFocus(false);
    session->SetSessionInfoAdvancedFeatureFlag(100, false);
    session->GetSessionInfoAdvancedFeatureFlag(100);
    session->SetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_LOCK_CURSOR, false);
    manager_->UpdateWindowFlagsForLockCursor(session, windowInfo);
    EXPECT_EQ(windowInfo.flags, 0);

    session->SetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_LOCK_CURSOR, true);
    manager_->UpdateWindowFlagsForLockCursor(session, windowInfo);
    EXPECT_FALSE(session->GetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_LOCK_CURSOR));

    session->UpdateFocus(true);
    session->SetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_LOCK_CURSOR, false);
    manager_->UpdateWindowFlagsForLockCursor(session, windowInfo);
    EXPECT_EQ(windowInfo.flags, 0);

    session->SetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_LOCK_CURSOR, true);
    session->SetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_CURSOR_FOLLOW_MOVEMENT, false);
    manager_->UpdateWindowFlagsForLockCursor(session, windowInfo);
    EXPECT_EQ(windowInfo.flags, MMI::WindowInputPolicy::FLAG_POINTER_LOCKED);

    windowInfo.flags = 0;
    session->SetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_LOCK_CURSOR, true);
    session->SetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_CURSOR_FOLLOW_MOVEMENT, true);
    manager_->UpdateWindowFlagsForLockCursor(session, windowInfo);
    EXPECT_EQ(windowInfo.flags, MMI::WindowInputPolicy::FLAG_POINTER_CONFINED);
}

/**
 * @tc.name: UpdateWindowFlagsForLockCursorInDrag
 * @tc.desc: UpdateWindowFlagsForLockCursorInDrag
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, UpdateWindowFlagsForLockCursorInDrag, TestSize.Level2)
{
    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->persistentId_ = 5;
    session->UpdateFocus(true);
    sptr<MoveDragController> moveDragController;
    moveDragController = sptr<MoveDragController>::MakeSptr(session);
    session->moveDragController_ = moveDragController;
    session->SetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_LOCK_CURSOR, true);
    session->SetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_CURSOR_FOLLOW_MOVEMENT, true);
    moveDragController->SetStartMoveFlag(false);
    EXPECT_FALSE(moveDragController->GetStartMoveFlag());
    EXPECT_FALSE(session->IsDragMoving());
    moveDragController->SetStartDragFlag(false);
    EXPECT_FALSE(moveDragController->GetStartDragFlag());
    EXPECT_FALSE(session->IsDragZooming());

    MMI::WindowInfo windowInfo;
    windowInfo.flags = 0;
    session->SetSessionInfoCursorDragFlag(true);
    manager_->UpdateWindowFlagsForLockCursor(session, windowInfo);
    manager_->UpdateWindowFlagsForLockCursor(session, windowInfo);
    EXPECT_EQ(windowInfo.flags, 0);

    windowInfo.flags = 0;
    moveDragController->hasPointDown_ = true;
    moveDragController->SetStartMoveFlag(true);
    EXPECT_TRUE(moveDragController->GetStartMoveFlag());
    EXPECT_TRUE(session->IsDragMoving());
    manager_->UpdateWindowFlagsForLockCursor(session, windowInfo);
    EXPECT_EQ(windowInfo.flags, 0);

    windowInfo.flags = 0;
    moveDragController->SetStartMoveFlag(false);
    EXPECT_FALSE(moveDragController->GetStartMoveFlag());
    EXPECT_FALSE(session->IsDragMoving());
    moveDragController->SetStartDragFlag(true);
    EXPECT_TRUE(moveDragController->GetStartDragFlag());
    EXPECT_TRUE(session->IsDragZooming());
    manager_->UpdateWindowFlagsForLockCursor(session, windowInfo);
    EXPECT_EQ(windowInfo.flags, 0);
}

/**
 * @tc.name: UpdateWindowFlagsForReceiveDragEventEnabled
 * @tc.desc: UpdateWindowFlagsForReceiveDragEventEnabled
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, UpdateWindowFlagsForReceiveDragEventEnabled, TestSize.Level2)
{
    MMI::WindowInfo windowInfo;
    manager_->UpdateWindowFlagsForReceiveDragEventEnabled(nullptr, windowInfo);
    EXPECT_EQ(windowInfo.flags, 0);

    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->persistentId_ = 5;
    session->SetSessionInfoAdvancedFeatureFlag(100, false);
    session->GetSessionInfoAdvancedFeatureFlag(100);
    session->SetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_RECEIVE_DRAG_EVENT, false);
    manager_->UpdateWindowFlagsForReceiveDragEventEnabled(session, windowInfo);
    EXPECT_EQ(windowInfo.flags, 0);

    windowInfo.flags = 0;
    session->SetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_RECEIVE_DRAG_EVENT, true);
    manager_->UpdateWindowFlagsForReceiveDragEventEnabled(session, windowInfo);
    EXPECT_NE(windowInfo.flags, 0);
}

/**
 * @tc.name: UpdateWindowFlagsForWindowSeparation
 * @tc.desc: UpdateWindowFlagsForWindowSeparation
 * @tc.type: FUNC
 */
HWTEST_F(SceneSessionDirtyManagerTest2, UpdateWindowFlagsForWindowSeparation, TestSize.Level2)
{
    MMI::WindowInfo windowInfo;
    manager_->UpdateWindowFlagsForReceiveDragEventEnabled(nullptr, windowInfo);
    EXPECT_EQ(windowInfo.flags, 0);

    SessionInfo info;
    sptr<SceneSession> session = sptr<SceneSession>::MakeSptr(info, nullptr);
    session->persistentId_ = 5;
    session->SetSessionInfoAdvancedFeatureFlag(100, false);
    session->GetSessionInfoAdvancedFeatureFlag(100);
    session->SetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_WINDOW_SEPARATION_TOUCH_ENABLED, false);
    manager_->UpdateWindowFlagsForWindowSeparation(session, windowInfo);
    EXPECT_EQ(windowInfo.flags, 0);

    windowInfo.flags = 0;
    session->SetSessionInfoAdvancedFeatureFlag(ADVANCED_FEATURE_BIT_WINDOW_SEPARATION_TOUCH_ENABLED, true);
    manager_->UpdateWindowFlagsForWindowSeparation(session, windowInfo);
    EXPECT_NE(windowInfo.flags, 0);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
