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
#include <pointer_event.h>
#include <ui/rs_surface_node.h>

#include "key_event.h"
#include "mock/mock_session_stage.h"
#include "mock/mock_window_event_channel.h"
#include "mock/mock_pattern_detach_callback.h"
#include "session/host/include/extension_session.h"
#include "session/host/include/move_drag_controller.h"
#include "session/host/include/scene_session.h"
#include "session_manager/include/scene_session_manager.h"
#include "session/host/include/session.h"
#include "session_info.h"
#include "wm_common.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
const std::string UNDEFINED = "undefined";
constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowSessionTest4"};
}

class WindowSessionTest4 : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    int32_t GetTaskCount();
    sptr<SceneSessionManager> ssm_;

private:
    RSSurfaceNode::SharedPtr CreateRSSurfaceNode();
    sptr<Session> session_ = nullptr;
    static constexpr uint32_t waitSyncInNs_ = 500000;
};

void WindowSessionTest4::SetUpTestCase()
{
}

void WindowSessionTest4::TearDownTestCase()
{
}

void WindowSessionTest4::SetUp()
{
    SessionInfo info;
    info.abilityName_ = "testSession1";
    info.moduleName_ = "testSession2";
    info.bundleName_ = "testSession4";
    session_ = sptr<Session>::MakeSptr(info);
    session_->surfaceNode_ = CreateRSSurfaceNode();
    EXPECT_NE(nullptr, session_);
    ssm_ = sptr<SceneSessionManager>::MakeSptr();
    session_->SetEventHandler(ssm_->taskScheduler_->GetEventHandler(), ssm_->eventHandler_);
    auto isScreenLockedCallback = [this]() {
        return ssm_->IsScreenLocked();
    };
    session_->RegisterIsScreenLockedCallback(isScreenLockedCallback);
}

void WindowSessionTest4::TearDown()
{
    session_ = nullptr;
    usleep(waitSyncInNs_);
}

RSSurfaceNode::SharedPtr WindowSessionTest4::CreateRSSurfaceNode()
{
    struct RSSurfaceNodeConfig rsSurfaceNodeConfig;
    rsSurfaceNodeConfig.SurfaceNodeName = "WindowSessionTest4SurfaceNode";
    auto surfaceNode = RSSurfaceNode::Create(rsSurfaceNodeConfig);
    if (surfaceNode == nullptr) {
        GTEST_LOG_(INFO) << "WindowSessionTest4::CreateRSSurfaceNode surfaceNode is nullptr";
    }
    return surfaceNode;
}

int32_t WindowSessionTest4::GetTaskCount()
{
    std::string dumpInfo = session_->handler_->GetEventRunner()->GetEventQueue()->DumpCurrentQueueSize();
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
 * @tc.name: SetShowRecent001
 * @tc.desc: Exist detect task when in recent.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetShowRecent001, Function | SmallTest | Level2)
{
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    auto task = [](){};
    int64_t delayTime = 3000;
    session_->handler_->PostTask(task, taskName, delayTime);
    int32_t beforeTaskNum = GetTaskCount();

    session_->SetShowRecent(true);
    ASSERT_EQ(beforeTaskNum, GetTaskCount());
    session_->handler_->RemoveTask(taskName);
}

/**
 * @tc.name: SetShowRecent002
 * @tc.desc: SetShowRecent:showRecent is false, showRecent_ is false.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetShowRecent002, Function | SmallTest | Level2)
{
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    auto task = [](){};
    int64_t delayTime = 3000;
    session_->handler_->PostTask(task, taskName, delayTime);
    session_->showRecent_ = false;
    int32_t beforeTaskNum = GetTaskCount();

    session_->SetShowRecent(false);
    ASSERT_EQ(beforeTaskNum, GetTaskCount());
    session_->handler_->RemoveTask(taskName);
}

/**
 * @tc.name: SetShowRecent003
 * @tc.desc: SetShowRecent:showRecent is false, showRecent_ is true, detach task.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetShowRecent003, Function | SmallTest | Level2)
{
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    auto task = [](){};
    int64_t delayTime = 3000;
    session_->handler_->PostTask(task, taskName, delayTime);
    session_->showRecent_ = true;
    session_->isAttach_ = false;
    int32_t beforeTaskNum = GetTaskCount();

    session_->SetShowRecent(false);
    ASSERT_EQ(beforeTaskNum, GetTaskCount());
    session_->handler_->RemoveTask(taskName);
}

/**
 * @tc.name: SetShowRecent004
 * @tc.desc: SetShowRecent
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetShowRecent004, Function | SmallTest | Level2)
{
    session_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ssm_->SetScreenLocked(false);
    sleep(1);

    session_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);

    bool showRecent = false;
    session_->showRecent_ = true;
    session_->SetAttachState(true);
    session_->SetShowRecent(showRecent);
    ASSERT_EQ(session_->GetShowRecent(), showRecent);
}

/**
 * @tc.name: CreateDetectStateTask001
 * @tc.desc: Create detection task when there are no pre_existing tasks.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, CreateDetectStateTask001, Function | SmallTest | Level2)
{
    session_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    DetectTaskInfo detectTaskInfo;
    detectTaskInfo.taskState = DetectTaskState::NO_TASK;
    int32_t beforeTaskNum = GetTaskCount();
    session_->SetDetectTaskInfo(detectTaskInfo);
    session_->CreateDetectStateTask(false, WindowMode::WINDOW_MODE_FULLSCREEN);

    ASSERT_EQ(beforeTaskNum + 1, GetTaskCount());
    ASSERT_EQ(DetectTaskState::DETACH_TASK, session_->GetDetectTaskInfo().taskState);
    session_->handler_->RemoveTask(taskName);

    session_->showRecent_ = true;
    session_->CreateDetectStateTask(false, WindowMode::WINDOW_MODE_FULLSCREEN);
}

/**
 * @tc.name: CreateDetectStateTask002
 * @tc.desc: Detect state when window mode changed.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, CreateDetectStateTask002, Function | SmallTest | Level2)
{
    session_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    auto task = [](){};
    int64_t delayTime = 3000;
    session_->handler_->PostTask(task, taskName, delayTime);
    int32_t beforeTaskNum = GetTaskCount();

    DetectTaskInfo detectTaskInfo;
    detectTaskInfo.taskState = DetectTaskState::DETACH_TASK;
    detectTaskInfo.taskWindowMode = WindowMode::WINDOW_MODE_FULLSCREEN;
    session_->SetDetectTaskInfo(detectTaskInfo);
    session_->CreateDetectStateTask(true, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);

    ASSERT_EQ(beforeTaskNum - 1, GetTaskCount());
    ASSERT_EQ(DetectTaskState::NO_TASK, session_->GetDetectTaskInfo().taskState);
    ASSERT_EQ(WindowMode::WINDOW_MODE_UNDEFINED, session_->GetDetectTaskInfo().taskWindowMode);
    session_->handler_->RemoveTask(taskName);

    session_->showRecent_ = true;
    session_->CreateDetectStateTask(false, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
}

/**
 * @tc.name: CreateDetectStateTask003
 * @tc.desc: Detect sup and down tree tasks for the same type.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, CreateDetectStateTask003, Function | SmallTest | Level2)
{
    session_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    DetectTaskInfo detectTaskInfo;
    detectTaskInfo.taskState = DetectTaskState::DETACH_TASK;
    detectTaskInfo.taskWindowMode = WindowMode::WINDOW_MODE_FULLSCREEN;
    session_->SetDetectTaskInfo(detectTaskInfo);
    session_->CreateDetectStateTask(true, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
    ASSERT_EQ(DetectTaskState::NO_TASK, session_->GetDetectTaskInfo().taskState);
    session_->handler_->RemoveTask(taskName);

    session_->showRecent_ = true;
    session_->CreateDetectStateTask(false, WindowMode::WINDOW_MODE_SPLIT_SECONDARY);
}

/**
 * @tc.name: CreateDetectStateTask004
 * @tc.desc: Detection tasks under the same window mode.
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, CreateDetectStateTask004, Function | SmallTest | Level2)
{
    session_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    DetectTaskInfo detectTaskInfo;
    int32_t beforeTaskNum = GetTaskCount();
    detectTaskInfo.taskState = DetectTaskState::DETACH_TASK;
    detectTaskInfo.taskWindowMode = WindowMode::WINDOW_MODE_FULLSCREEN;
    session_->SetDetectTaskInfo(detectTaskInfo);
    session_->CreateDetectStateTask(true, WindowMode::WINDOW_MODE_FULLSCREEN);

    ASSERT_EQ(beforeTaskNum + 1, GetTaskCount());
    ASSERT_EQ(DetectTaskState::ATTACH_TASK, session_->GetDetectTaskInfo().taskState);
    session_->handler_->RemoveTask(taskName);

    session_->showRecent_ = true;
    session_->CreateDetectStateTask(false, WindowMode::WINDOW_MODE_FULLSCREEN);
}

/**
 * @tc.name: GetAttachState001
 * @tc.desc: GetAttachState001
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, GetAttachState001, Function | SmallTest | Level2)
{
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    session_->SetAttachState(false);
    bool isAttach = session_->GetAttachState();
    ASSERT_EQ(false, isAttach);
    session_->handler_->RemoveTask(taskName);
}

/**
 * @tc.name: ResetSessionConnectState
 * @tc.desc: ResetSessionConnectState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, ResetSessionConnectState, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->ResetSessionConnectState();
    ASSERT_EQ(session_->state_, SessionState::STATE_DISCONNECT);
    ASSERT_EQ(session_->GetCallingPid(), -1);
}

/**
 * @tc.name: ResetIsActive
 * @tc.desc: ResetIsActive
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, ResetIsActive, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->ResetIsActive();
    ASSERT_EQ(session_->isActive_, false);
}

/**
 * @tc.name: PostExportTask02
 * @tc.desc: PostExportTask
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, PostExportTask02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    std::string name = "sessionExportTask";
    auto task = [](){};
    int64_t delayTime = 0;

    session_->PostExportTask(task, name, delayTime);
    auto result = session_->GetBufferAvailable();
    ASSERT_EQ(result, false);

    sptr<SceneSessionManager> sceneSessionManager = sptr<SceneSessionManager>::MakeSptr();
    session_->SetEventHandler(sceneSessionManager->taskScheduler_->GetEventHandler(),
        sceneSessionManager->eventHandler_);
    session_->PostExportTask(task, name, delayTime);
    auto result2 = session_->GetBufferAvailable();
    ASSERT_EQ(result2, false);
}

/**
 * @tc.name: SetLeashWinSurfaceNode02
 * @tc.desc: SetLeashWinSurfaceNode
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetLeashWinSurfaceNode02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->leashWinSurfaceNode_ = WindowSessionTest4::CreateRSSurfaceNode();
    session_->SetLeashWinSurfaceNode(nullptr);

    session_->leashWinSurfaceNode_ = nullptr;
    session_->SetLeashWinSurfaceNode(nullptr);
    auto result = session_->GetBufferAvailable();
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: GetCloseAbilityWantAndClean
 * @tc.desc: GetCloseAbilityWantAndClean
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, GetCloseAbilityWantAndClean, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    AAFwk::Want outWant;
    session_->sessionInfo_.closeAbilityWant = std::make_shared<AAFwk::Want>();
    session_->GetCloseAbilityWantAndClean(outWant);

    session_->sessionInfo_.closeAbilityWant = nullptr;
    session_->GetCloseAbilityWantAndClean(outWant);
    auto result = session_->GetBufferAvailable();
    ASSERT_EQ(result, false);
}

/**
 * @tc.name: SetScreenId02
 * @tc.desc: SetScreenId Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetScreenId02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    uint64_t screenId = 0;
    session_->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    session_->SetScreenId(screenId);
    ASSERT_EQ(0, session_->sessionInfo_.screenId_);
}

/**
 * @tc.name: SetSessionState
 * @tc.desc: SetSessionState
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetSessionState, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    SessionState state03 = SessionState::STATE_CONNECT;
    session_->SetSessionState(state03);
    ASSERT_EQ(state03, session_->state_);
}

/**
 * @tc.name: SetFocusable03
 * @tc.desc: SetFocusable
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetFocusable03, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->isFocused_ = true;
    session_->property_->focusable_ = false;
    bool isFocusable = true;

    auto result = session_->SetFocusable(isFocusable);
    ASSERT_EQ(result, WSError::WS_OK);
    ASSERT_EQ(session_->GetFocusable(), true);
}

/**
 * @tc.name: GetFocused
 * @tc.desc: GetFocused Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, GetFocused, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    bool result = session_->GetFocused();
    ASSERT_EQ(result, false);

    session_->isFocused_ = true;
    bool result2 = session_->GetFocused();
    ASSERT_EQ(result2, true);
}

/**
 * @tc.name: UpdatePointerArea
 * @tc.desc: UpdatePointerArea Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, UpdatePointerArea, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    WSRect rect = { 0, 0, 0, 0 };
    session_->preRect_ = rect;
    session_->UpdatePointerArea(rect);
    ASSERT_EQ(session_->GetFocused(), false);
}

/**
 * @tc.name: UpdateSizeChangeReason02
 * @tc.desc: UpdateSizeChangeReason Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, UpdateSizeChangeReason02, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    SizeChangeReason reason = SizeChangeReason::UNDEFINED;
    WSError result = session_->UpdateSizeChangeReason(reason);
    ASSERT_EQ(result, WSError::WS_DO_NOTHING);
}

/**
 * @tc.name: UpdateDensity
 * @tc.desc: UpdateDensity Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, UpdateDensity, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);

    session_->state_ = SessionState::STATE_DISCONNECT;
    ASSERT_FALSE(session_->IsSessionValid());
    WSError result = session_->UpdateDensity();
    ASSERT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);

    session_->state_ = SessionState::STATE_CONNECT;
    ASSERT_TRUE(session_->IsSessionValid());
    session_->sessionStage_ = nullptr;
    WSError result02 = session_->UpdateDensity();
    ASSERT_EQ(result02, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateSizeChangeReason
 * @tc.desc: UpdateSizeChangeReason UpdateDensity
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, UpdateSizeChangeReason, Function | SmallTest | Level2)
{
    SizeChangeReason reason = SizeChangeReason{1};
    ASSERT_EQ(session_->UpdateSizeChangeReason(reason), WSError::WS_OK);
}

/**
 * @tc.name: SetPendingSessionActivationEventListener
 * @tc.desc: SetPendingSessionActivationEventListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetPendingSessionActivationEventListener, Function | SmallTest | Level2)
{
    int resultValue = 0;
    session_->SetPendingSessionActivationEventListener([&resultValue](const SessionInfo& info) {
        resultValue = 1;
    });
    usleep(waitSyncInNs_);
    session_->SetTerminateSessionListener([&resultValue](const SessionInfo& info) {
        resultValue = 2;
    });
    usleep(waitSyncInNs_);
    LifeCycleTaskType taskType = LifeCycleTaskType{0};
    session_->RemoveLifeCycleTask(taskType);
    ASSERT_EQ(resultValue, 0);
}

/**
 * @tc.name: SetSessionIcon
 * @tc.desc: SetSessionIcon UpdateDensity
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetSessionIcon, Function | SmallTest | Level2)
{
    std::shared_ptr<Media::PixelMap> icon;
    session_->SetSessionIcon(icon);
    ASSERT_EQ(session_->Clear(), WSError::WS_OK);
    session_->SetSessionSnapshotListener(nullptr);
    NotifyPendingSessionActivationFunc func = [](const SessionInfo& info) {};
    session_->pendingSessionActivationFunc_ = func;
    ASSERT_EQ(session_->PendingSessionToForeground(), WSError::WS_OK);

    session_->scenePersistence_ = sptr<ScenePersistence>::MakeSptr("SetSessionIcon", 1);
    session_->updateSessionIconFunc_ = nullptr;
    ASSERT_EQ(WSError::WS_OK, session_->SetSessionIcon(icon));

    NofitySessionIconUpdatedFunc func2 = [](const std::string& iconPath) {};
    session_->updateSessionIconFunc_ = func2;
    ASSERT_EQ(WSError::WS_OK, session_->SetSessionIcon(icon));

    NotifyTerminateSessionFuncNew func3 = [](const SessionInfo& info, bool needStartCaller, bool isFromBroker) {};
    session_->terminateSessionFuncNew_ = func3;
    ASSERT_EQ(WSError::WS_OK, session_->Clear());
}

/**
 * @tc.name: SetSessionExceptionListener
 * @tc.desc: SetSessionExceptionListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetSessionExceptionListener, Function | SmallTest | Level2)
{
    session_->SetSessionExceptionListener(nullptr, true);
    session_->SetSessionExceptionListener([](const SessionInfo& info, bool removeSession, bool startFail) {}, true);
    usleep(waitSyncInNs_);
    ASSERT_NE(nullptr, session_->jsSceneSessionExceptionFunc_);
}

/**
 * @tc.name: SetRaiseToAppTopForPointDownFunc
 * @tc.desc: SetRaiseToAppTopForPointDownFunc Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetRaiseToAppTopForPointDownFunc, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetRaiseToAppTopForPointDownFunc(nullptr);

    NotifyRaiseToTopForPointDownFunc func = []() {};
    session_->raiseToTopForPointDownFunc_ = func;
    session_->RaiseToAppTopForPointDown();
    session_->HandlePointDownDialog();
    session_->ClearDialogVector();

    session_->SetBufferAvailableChangeListener(nullptr);
    session_->UnregisterSessionChangeListeners();
    session_->SetSessionStateChangeNotifyManagerListener(nullptr);
    session_->SetSessionInfoChangeNotifyManagerListener(nullptr);
    session_->NotifyFocusStatus(true);

    session_->SetRequestFocusStatusNotifyManagerListener(nullptr);
    session_->SetNotifyUIRequestFocusFunc(nullptr);
    session_->SetNotifyUILostFocusFunc(nullptr);
    session_->UnregisterSessionChangeListeners();

    NotifyPendingSessionToBackgroundForDelegatorFunc func2 = [](const SessionInfo& info, bool shouldBackToCaller) {};
    session_->pendingSessionToBackgroundForDelegatorFunc_ = func2;
    ASSERT_EQ(WSError::WS_OK, session_->PendingSessionToBackgroundForDelegator(true));
}

/**
 * @tc.name: NotifyCloseExistPipWindow
 * @tc.desc: check func NotifyCloseExistPipWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, NotifyCloseExistPipWindow, Function | SmallTest | Level2)
{
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    ManagerState key = ManagerState{0};
    session_->GetStateFromManager(key);
    session_->NotifyUILostFocus();

    session_->lostFocusFunc_ = []() {};
    session_->NotifyUILostFocus();

    session_->SetSystemSceneBlockingFocus(true);
    session_->GetBlockingFocus();
    session_->sessionStage_ = mockSessionStage;
    EXPECT_CALL(*(mockSessionStage), NotifyCloseExistPipWindow()).Times(1).WillOnce(Return(WSError::WS_OK));
    ASSERT_EQ(WSError::WS_OK, session_->NotifyCloseExistPipWindow());
    session_->sessionStage_ = nullptr;
    ASSERT_EQ(WSError::WS_ERROR_NULLPTR, session_->NotifyCloseExistPipWindow());
}

/**
 * @tc.name: SetUseStartingWindowAboveLocked
 * @tc.desc: SetUseStartingWindowAboveLocked Test
 * @tc.type: FUNC
 */
 HWTEST_F(WindowSessionTest4, SetUseStartingWindowAboveLocked, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->useStartingWindowAboveLocked_ = false;
    ASSERT_EQ(session_->useStartingWindowAboveLocked_, false);
    session_->SetUseStartingWindowAboveLocked(true);
    ASSERT_EQ(session_->UseStartingWindowAboveLocked(), true);
}

/**
 * @tc.name: SetSystemConfig
 * @tc.desc: SetSystemConfig Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetSystemConfig, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    SystemSessionConfig systemConfig;
    session_->SetSystemConfig(systemConfig);
    float snapshotScale = 0.5;
    session_->SetSnapshotScale(snapshotScale);
    session_->ProcessBackEvent();
    session_->NotifyOccupiedAreaChangeInfo(nullptr);
    session_->UpdateMaximizeMode(true);
    ASSERT_EQ(session_->GetZOrder(), 0);

    session_->SetUINodeId(0);
    session_->GetUINodeId();
    session_->SetShowRecent(true);
    session_->GetShowRecent();
    session_->SetBufferAvailable(true);

    session_->SetNeedSnapshot(true);
    session_->SetFloatingScale(0.5);
    ASSERT_EQ(session_->GetFloatingScale(), 0.5f);
    session_->SetScale(50, 100, 50, 100);
    session_->GetScaleX();
    session_->GetScaleY();
    session_->GetPivotX();
    session_->GetPivotY();
    session_->SetSCBKeepKeyboard(true);
    session_->GetSCBKeepKeyboardFlag();
    ASSERT_EQ(WSError::WS_OK, session_->MarkProcessed(11));
}

/**
 * @tc.name: SetOffset
 * @tc.desc: SetOffset Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetOffset, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetOffset(50, 100);
    session_->GetOffsetX();
    session_->GetOffsetY();
    WSRectF bounds;
    session_->SetBounds(bounds);
    session_->GetBounds();
    session_->UpdateTitleInTargetPos(true, 100);
    session_->SetNotifySystemSessionPointerEventFunc(nullptr);
    session_->SetNotifySystemSessionKeyEventFunc(nullptr);
    ASSERT_EQ(session_->GetBufferAvailable(), false);
}

/**
 * @tc.name: SetBackPressedListenser
 * @tc.desc: SetBackPressedListenser Test
 type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetBackPressedListenser, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    int32_t result = 0;
    session_->SetBackPressedListenser([&result](const bool needMoveToBackground) {
        result = 1;
    });
    usleep(waitSyncInNs_);
    session_->backPressedFunc_(true);
    ASSERT_EQ(result, 1);
}

/**
 * @tc.name: SetUpdateSessionIconListener
 * @tc.desc: SetUpdateSessionIconListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetUpdateSessionIconListener, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    WLOGFI("SetUpdateSessionIconListener begin!");

    session_->SetUpdateSessionIconListener(session_->updateSessionIconFunc_);

    WLOGFI("SetUpdateSessionIconListener end!");
}

/**
 * @tc.name: NotifyContextTransparent
 * @tc.desc: NotifyContextTransparent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, NotifyContextTransparent, Function | SmallTest | Level2)
{
    WLOGFI("NotifyContextTransparent begin!");
    ASSERT_NE(session_, nullptr);

    NotifyContextTransparentFunc contextTransparentFunc = session_->contextTransparentFunc_;
    if (contextTransparentFunc == nullptr) {
        contextTransparentFunc = [](){};
    }
    session_->contextTransparentFunc_ = nullptr;
    session_->NotifyContextTransparent();

    session_->SetContextTransparentFunc(contextTransparentFunc);
    session_->NotifyContextTransparent();

    WLOGFI("NotifyContextTransparent end!");
}

/**
 * @tc.name: NotifySessionInfoLockedStateChange
 * @tc.desc: NotifySessionInfoLockedStateChange Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, NotifySessionInfoLockedStateChange, Function | SmallTest | Level2)
{
    WLOGFI("NotifySessionInfoLockedStateChange begin!");
    ASSERT_NE(session_, nullptr);

    NotifySessionInfoLockedStateChangeFunc sessionInfoLockedStateChangeFunc =
        session_->sessionInfoLockedStateChangeFunc_;
    if (sessionInfoLockedStateChangeFunc == nullptr) {
        sessionInfoLockedStateChangeFunc = [](const bool lockedState) {};
    }
    session_->sessionInfoLockedStateChangeFunc_ = nullptr;
    session_->NotifySessionInfoLockedStateChange(true);

    session_->SetSessionInfoLockedStateChangeListener(sessionInfoLockedStateChangeFunc);
    session_->NotifySessionInfoLockedStateChange(true);

    WLOGFI("NotifySessionInfoLockedStateChange end!");
}

/**
 * @tc.name: GetMainSession
 * @tc.desc: GetMainSession Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, GetMainSession, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    SessionInfo info;
    info.abilityName_ = "getMainSession";
    info.moduleName_ = "getMainSession";
    info.bundleName_ = "getMainSession";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    ASSERT_NE(session, nullptr);
    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    EXPECT_EQ(session, session->GetMainSession());

    sptr<Session> subSession = sptr<Session>::MakeSptr(info);
    ASSERT_NE(subSession, nullptr);
    subSession->SetParentSession(session);
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(session, subSession->GetMainSession());

    sptr<Session> subSubSession = sptr<Session>::MakeSptr(info);
    ASSERT_NE(subSubSession, nullptr);
    subSubSession->SetParentSession(subSession);
    subSubSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(session, subSubSession->GetMainSession());
}

/**
 * @tc.name: GetMainOrFloatSession
 * @tc.desc: GetMainOrFloatSession Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, GetMainOrFloatSession, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    SessionInfo info;
    info.abilityName_ = "GetMainOrFloatSession";
    info.moduleName_ = "GetMainOrFloatSession";
    info.bundleName_ = "GetMainOrFloatSession";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    session_->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    EXPECT_EQ(session, session->GetMainOrFloatSession());

    sptr<Session> floatSession = sptr<Session>::MakeSptr(info);
    floatSession->SetParentSession(session);
    floatSession->property_->SetWindowType(WindowType::WINDOW_TYPE_FLOAT);
    EXPECT_EQ(floatSession, floatSession->GetMainOrFloatSession());

    sptr<Session> subSession = sptr<Session>::MakeSptr(info);
    subSession->SetParentSession(floatSession);
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(floatSession, subSession->GetMainOrFloatSession());
}

/**
 * @tc.name: IsSupportDetectWindow
 * @tc.desc: IsSupportDetectWindow Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, IsSupportDetectWindow, Function | SmallTest | Level2)
{
    session_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    ssm_->SetScreenLocked(true);
    sleep(1);
    bool ret = session_->IsSupportDetectWindow(true);
    ASSERT_EQ(ret, false);

    ssm_->SetScreenLocked(false);
    sleep(1);
    session_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    ret = session_->IsSupportDetectWindow(true);
    ASSERT_EQ(ret, false);

    ssm_->SetScreenLocked(false);
    sleep(1);
    session_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    session_->systemConfig_.windowUIType_ = WindowUIType::PC_WINDOW;
    ret = session_->IsSupportDetectWindow(false);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: ShouldCreateDetectTask
 * @tc.desc: ShouldCreateDetectTask Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, ShouldCreateDetectTask, Function | SmallTest | Level2)
{
    DetectTaskInfo detectTaskInfo;
    detectTaskInfo.taskState = DetectTaskState::ATTACH_TASK;
    detectTaskInfo.taskWindowMode = WindowMode::WINDOW_MODE_FULLSCREEN;
    session_->SetDetectTaskInfo(detectTaskInfo);
    bool ret = session_->ShouldCreateDetectTask(true, WindowMode::WINDOW_MODE_UNDEFINED);
    ASSERT_EQ(ret, true);
    detectTaskInfo.taskState = DetectTaskState::DETACH_TASK;
    session_->SetDetectTaskInfo(detectTaskInfo);
    ret = session_->ShouldCreateDetectTask(false, WindowMode::WINDOW_MODE_UNDEFINED);
    ASSERT_EQ(ret, true);
    ret = session_->ShouldCreateDetectTask(true, WindowMode::WINDOW_MODE_UNDEFINED);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: ShouldCreateDetectTaskInRecent
 * @tc.desc: ShouldCreateDetectTaskInRecent Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, ShouldCreateDetectTaskInRecent, Function | SmallTest | Level2)
{
    bool ret = session_->ShouldCreateDetectTaskInRecent(true, true, true);
    ASSERT_EQ(ret, false);
    ret = session_->ShouldCreateDetectTaskInRecent(false, true, true);
    ASSERT_EQ(ret, true);
    ret = session_->ShouldCreateDetectTaskInRecent(false, true, false);
    ASSERT_EQ(ret, false);
    ret = session_->ShouldCreateDetectTaskInRecent(false, false, false);
    ASSERT_EQ(ret, false);
}

/**
 * @tc.name: CreateWindowStateDetectTask
 * @tc.desc: CreateWindowStateDetectTask Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, CreateWindowStateDetectTask, Function | SmallTest | Level2)
{
    auto isScreenLockedCallback = [this]() { return ssm_->IsScreenLocked(); };
    session_->RegisterIsScreenLockedCallback(isScreenLockedCallback);
    session_->SetSessionState(SessionState::STATE_CONNECT);
    bool isAttach = true;
    session_->CreateWindowStateDetectTask(isAttach, WindowMode::WINDOW_MODE_UNDEFINED);
    ASSERT_EQ(isAttach, true);

    session_->handler_ = nullptr;
    session_->CreateWindowStateDetectTask(false, WindowMode::WINDOW_MODE_UNDEFINED);
    ASSERT_EQ(session_->handler_, nullptr);
}

/**
 * @tc.name: SetOffset01
 * @tc.desc: SetOffset Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetOffset01, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    session_->SetOffset(0, 0);
    ASSERT_EQ(session_->GetOffsetX(), 0);
}

/**
 * @tc.name: GetIsMidScene
 * @tc.desc: GetIsMidScene Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, GetIsMidScene, Function | SmallTest | Level2)
{
    ASSERT_NE(session_, nullptr);
    bool isMidScene = false;
    auto result = session_->GetIsMidScene(isMidScene);
    ASSERT_EQ(result, WSError::WS_OK);
    ASSERT_EQ(isMidScene, false);
}
}
} // namespace Rosen
} // namespace OHOS
