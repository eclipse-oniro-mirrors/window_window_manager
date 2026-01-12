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

#include "iremote_object_mocker.h"
#include "key_event.h"
#include "mock/mock_session.h"
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
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, HILOG_DOMAIN_WINDOW, "WindowSessionTest4" };
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }
} // namespace
namespace {
std::string g_logMsg;
void SessionTest4LogCallBack(const LogType type,
                             const LogLevel level,
                             const unsigned int domain,
                             const char* tag,
                             const char* msg)
{
    g_logMsg = msg;
}
} // namespace
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

void WindowSessionTest4::SetUpTestCase() {}

void WindowSessionTest4::TearDownTestCase() {}

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
    auto isScreenLockedCallback = [this]() { return ssm_->IsScreenLocked(); };
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
HWTEST_F(WindowSessionTest4, SetShowRecent001, TestSize.Level1)
{
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    auto task = []() {};
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
HWTEST_F(WindowSessionTest4, SetShowRecent002, TestSize.Level1)
{
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    auto task = []() {};
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
HWTEST_F(WindowSessionTest4, SetShowRecent003, TestSize.Level1)
{
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    auto task = []() {};
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
HWTEST_F(WindowSessionTest4, SetShowRecent004, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, CreateDetectStateTask001, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, CreateDetectStateTask002, TestSize.Level1)
{
    session_->systemConfig_.windowUIType_ = WindowUIType::PHONE_WINDOW;
    std::string taskName = "wms:WindowStateDetect" + std::to_string(session_->persistentId_);
    auto task = []() {};
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
HWTEST_F(WindowSessionTest4, CreateDetectStateTask003, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, CreateDetectStateTask004, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, GetAttachState001, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, ResetSessionConnectState, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, ResetIsActive, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, PostExportTask02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    std::string name = "sessionExportTask";
    auto task = []() {};
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
HWTEST_F(WindowSessionTest4, SetLeashWinSurfaceNode02, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, GetCloseAbilityWantAndClean, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, SetScreenId02, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, SetSessionState, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, SetFocusable03, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, GetFocused, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, UpdatePointerArea, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, UpdateSizeChangeReason02, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, UpdateDensity, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, UpdateSizeChangeReason, TestSize.Level1)
{
    SizeChangeReason reason = SizeChangeReason{ 1 };
    ASSERT_EQ(session_->UpdateSizeChangeReason(reason), WSError::WS_OK);
}

/**
 * @tc.name: SetPendingSessionActivationEventListener
 * @tc.desc: SetPendingSessionActivationEventListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetPendingSessionActivationEventListener, TestSize.Level1)
{
    int resultValue = 0;
    session_->SetPendingSessionActivationEventListener([&resultValue](const SessionInfo& info) { resultValue = 1; });
    usleep(waitSyncInNs_);
    session_->SetTerminateSessionListener([&resultValue](const SessionInfo& info) { resultValue = 2; });
    usleep(waitSyncInNs_);
    LifeCycleTaskType taskType = LifeCycleTaskType{ 0 };
    session_->RemoveLifeCycleTask(taskType);
    ASSERT_EQ(resultValue, 0);
}

/**
 * @tc.name: SetSessionIcon
 * @tc.desc: SetSessionIcon UpdateDensity
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetSessionIcon, TestSize.Level1)
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

    NotifyTerminateSessionFuncNew func3 = 
        [](const SessionInfo& info, bool needStartCaller, bool isFromBroker, bool isForceClean) {};
    session_->terminateSessionFuncNew_ = func3;
    ASSERT_EQ(WSError::WS_OK, session_->Clear());
}

/**
 * @tc.name: SetSessionExceptionListener
 * @tc.desc: SetSessionExceptionListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetSessionExceptionListener, TestSize.Level1)
{
    session_->SetSessionExceptionListener(nullptr, true);
    session_->SetSessionExceptionListener(
        [](const SessionInfo& info, const ExceptionInfo& exceptionInfo, bool startFail) {}, true);
    usleep(waitSyncInNs_);
    ASSERT_NE(nullptr, session_->jsSceneSessionExceptionFunc_);
}

/**
 * @tc.name: SetRaiseToAppTopForPointDownFunc
 * @tc.desc: SetRaiseToAppTopForPointDownFunc Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetRaiseToAppTopForPointDownFunc, TestSize.Level1)
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
    auto info = sptr<FocusNotifyInfo>::MakeSptr();
    session_->NotifyFocusStatus(info, true);

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
HWTEST_F(WindowSessionTest4, NotifyCloseExistPipWindow, TestSize.Level1)
{
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(mockSessionStage, nullptr);
    ManagerState key = ManagerState{ 0 };
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
HWTEST_F(WindowSessionTest4, SetUseStartingWindowAboveLocked, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, SetSystemConfig, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, SetOffset, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, SetBackPressedListenser, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    int32_t result = 0;
    session_->SetBackPressedListenser([&result](const bool needMoveToBackground) { result = 1; });
    usleep(waitSyncInNs_);
    session_->backPressedFunc_(true);
    ASSERT_EQ(result, 1);
}

/**
 * @tc.name: SetRestartAppListener
 * @tc.desc: SetRestartAppListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetRestartAppListener, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    int32_t result = 0;
    session_->SetRestartAppListener([&result](const SessionInfo& info, int32_t callingPid) {
        result = 1;
    });
    usleep(waitSyncInNs_);
    SessionInfo info;
    int32_t callingPid = 0;
    session_->restartAppFunc_(info, callingPid);
    ASSERT_EQ(result, 1);
}

/**
 * @tc.name: NotifyRestart
 * @tc.desc: NotifyRestart Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, NotifyRestart, TestSize.Level1)
{
    SessionInfo info;
    info.bundleName_ = "bundleName_";
    info.moduleName_ = "moduleName_";
    info.abilityName_ = "abilityName_";
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(info, nullptr);
    ASSERT_NE(nullptr, sceneSession);
    sceneSession->NotifyRestart();
    ASSERT_NE(nullptr, sceneSession);
}

/**
 * @tc.name: SetUpdateSessionIconListener
 * @tc.desc: SetUpdateSessionIconListener Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetUpdateSessionIconListener, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, NotifyContextTransparent, TestSize.Level1)
{
    WLOGFI("NotifyContextTransparent begin!");
    ASSERT_NE(session_, nullptr);

    NotifyContextTransparentFunc contextTransparentFunc = session_->contextTransparentFunc_;
    if (contextTransparentFunc == nullptr) {
        contextTransparentFunc = []() {};
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
HWTEST_F(WindowSessionTest4, NotifySessionInfoLockedStateChange, TestSize.Level1)
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
 * @tc.name: SetCallingSessionIdSessionListenser
 * @tc.desc: SetCallingSessionIdSessionListenser Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetCallingSessionIdSessionListenser, TestSize.Level1)
{
    WLOGFI("SetCallingSessionIdSessionListenser begin!");
    ASSERT_NE(session_, nullptr);
    ProcessCallingSessionIdChangeFunc func;
    session_->SetCallingSessionIdSessionListenser(std::move(func));
    WLOGFI("SetCallingSessionIdSessionListenser end!");
}

/**
 * @tc.name: GetMainSession
 * @tc.desc: GetMainSession Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, GetMainSession, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, GetMainOrFloatSession, TestSize.Level1)
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
 * @tc.name: IsAncestorsSession
 * @tc.desc: IsAncestorsSession Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, IsAncestorsSession, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "IsAncestorsSession";
    info.moduleName_ = "IsAncestorsSession";
    info.bundleName_ = "IsAncestorsSession";
    sptr<Session> session = sptr<Session>::MakeSptr(info);
    session->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    session->property_->SetPersistentId(1);

    sptr<Session> subSession = sptr<Session>::MakeSptr(info);
    subSession->SetParentSession(session);
    subSession->property_->SetPersistentId(2);
    subSession->property_->SetParentPersistentId(1);
    subSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(true, subSession->IsAncestorsSession(1));

    sptr<Session> subSubSession = sptr<Session>::MakeSptr(info);
    subSubSession->SetParentSession(subSession);
    subSubSession->property_->SetPersistentId(3);
    subSubSession->property_->SetParentPersistentId(2);
    subSubSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(true, subSubSession->IsAncestorsSession(1));
    EXPECT_EQ(true, subSubSession->IsAncestorsSession(2));
    EXPECT_EQ(false, subSubSession->IsAncestorsSession(3));
}

/**
 * @tc.name: IsSupportDetectWindow
 * @tc.desc: IsSupportDetectWindow Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, IsSupportDetectWindow, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, ShouldCreateDetectTask, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, ShouldCreateDetectTaskInRecent, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, CreateWindowStateDetectTask, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    auto isScreenLockedCallback = [this]() { return ssm_->IsScreenLocked(); };
    session_->RegisterIsScreenLockedCallback(isScreenLockedCallback);
    session_->SetSessionState(SessionState::STATE_CONNECT);
    bool isAttach = true;
    session_->CreateWindowStateDetectTask(isAttach, WindowMode::WINDOW_MODE_UNDEFINED);
    EXPECT_FALSE(g_errLog.find("Window attach state and session state mismatch, ") != std::string::npos);

    session_->handler_ = nullptr;
    session_->CreateWindowStateDetectTask(false, WindowMode::WINDOW_MODE_UNDEFINED);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: SetOffset01
 * @tc.desc: SetOffset Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetOffset01, TestSize.Level1)
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
HWTEST_F(WindowSessionTest4, GetIsMidScene, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool isMidScene = false;
    auto result = session_->GetIsMidScene(isMidScene);
    ASSERT_EQ(result, WSError::WS_OK);
    ASSERT_EQ(isMidScene, false);
}

/**
 * @tc.name: GetWindowUIInfoForWindowInfo01
 * @tc.desc: GetWindowUIInfoForWindowInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, GetWindowUIInfoForWindowInfo01, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->SetVisibilityState(WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
    WSRect rect = { 0, 0, 100, 100 };
    sceneSession->SetSessionRect(rect);
    sceneSession->SetSessionGlobalRect(rect);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->GetSessionProperty()->SetDisplayId(0);

    WindowUIInfo windowUIInfo = sceneSession->GetWindowUIInfoForWindowInfo();
    ASSERT_EQ(windowUIInfo.visibilityState, sceneSession->GetVisibilityState());
}

/**
 * @tc.name: GetWindowDisplayInfoForWindowInfo01
 * @tc.desc: GetWindowDisplayInfoForWindowInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, GetWindowDisplayInfoForWindowInfo01, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->SetVisibilityState(WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
    WSRect rect = { 5, 0, 100, 100 };
    sceneSession->SetSessionRect(rect);
    sceneSession->SetSessionGlobalRect(rect);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    constexpr DisplayId SECOND_DISPLAY_ID = 11;
    sceneSession->GetSessionProperty()->SetDisplayId(SECOND_DISPLAY_ID);

    WindowDisplayInfo windowDisplayInfo = sceneSession->GetWindowDisplayInfoForWindowInfo();
    ASSERT_EQ(windowDisplayInfo.displayId, sceneSession->GetSessionProperty()->GetDisplayId());
}

/**
 * @tc.name: GetWindowLayoutInfoForWindowInfo01
 * @tc.desc: GetWindowLayoutInfoForWindowInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, GetWindowLayoutInfoForWindowInfo01, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->SetVisibilityState(WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
    WSRect rect = { 5, 0, 100, 100 };
    sceneSession->SetSessionRect(rect);
    sceneSession->SetSessionGlobalRect(rect);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->GetSessionProperty()->SetDisplayId(0);
    sceneSession->SetZOrder(100);

    WindowLayoutInfo windowLayoutInfo = sceneSession->GetWindowLayoutInfoForWindowInfo();
    ASSERT_EQ(windowLayoutInfo.rect.posX_, 5);
    ASSERT_EQ(windowLayoutInfo.rect.posY_, 0);
    ASSERT_EQ(windowLayoutInfo.rect.width_, 100);
    ASSERT_EQ(windowLayoutInfo.rect.height_, 100);
    ASSERT_EQ(windowLayoutInfo.zOrder, 100);
}

/**
 * @tc.name: GetWindowMetaInfoForWindowInfo01
 * @tc.desc: GetWindowMetaInfoForWindowInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, GetWindowMetaInfoForWindowInfo01, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;
    sessionInfo.bundleName_ = "bundleName";
    sessionInfo.abilityName_ = "abilityName";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    sceneSession->GetSessionProperty()->SetWindowName("sceneSession");
    sceneSession->SetVisibilityState(WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
    WSRect rect = { 5, 0, 100, 100 };
    sceneSession->SetSessionRect(rect);
    sceneSession->SetSessionGlobalRect(rect);
    sceneSession->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession->GetSessionProperty()->SetDisplayId(0);
    sceneSession->callingPid_ = 123;
    sceneSession->UpdateWindowMode(WindowMode::WINDOW_MODE_FULLSCREEN);
    sceneSession->isMidScene_ = true;
    sceneSession->isFocused_ = true;
    SessionInfo sessionInfo1;
    sessionInfo1.isSystem_ = true;
    sessionInfo1.abilityName_ = "abilityName1";
    sptr<SceneSession> sceneSession1 = sptr<SceneSession>::MakeSptr(sessionInfo1, nullptr);
    sceneSession1->SetVisibilityState(WINDOW_VISIBILITY_STATE_TOTALLY_OCCUSION);
    rect = { 200, 0, 100, 100 };
    sceneSession1->SetSessionRect(rect);
    sceneSession1->SetSessionGlobalRect(rect);
    sceneSession1->SetSessionState(SessionState::STATE_FOREGROUND);
    sceneSession1->GetSessionProperty()->SetDisplayId(0);
    sceneSession1->SetParentSession(sceneSession);
    sceneSession1->property_->SetPrivacyMode(true);
    sceneSession1->surfaceNode_ = nullptr;
    sceneSession1->leashWinSurfaceNode_ = nullptr;

    WindowMetaInfo windowMetaInfo = sceneSession->GetWindowMetaInfoForWindowInfo();
    ASSERT_EQ(windowMetaInfo.windowId, sceneSession->GetWindowId());
    ASSERT_EQ(windowMetaInfo.windowName, sceneSession->GetSessionProperty()->GetWindowName());
    ASSERT_EQ(windowMetaInfo.bundleName, sceneSession->GetSessionInfo().bundleName_);
    ASSERT_EQ(windowMetaInfo.abilityName, sceneSession->GetSessionInfo().abilityName_);
    ASSERT_EQ(windowMetaInfo.pid, sceneSession->GetCallingPid());
    ASSERT_EQ(windowMetaInfo.windowType, WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ASSERT_EQ(windowMetaInfo.windowMode, WindowMode::WINDOW_MODE_FULLSCREEN);
    ASSERT_EQ(windowMetaInfo.isMidScene, true);
    ASSERT_EQ(windowMetaInfo.isFocused, true);
    WindowMetaInfo windowMetaInfo1 = sceneSession1->GetWindowMetaInfoForWindowInfo();
    ASSERT_EQ(windowMetaInfo1.windowName, sceneSession1->GetSessionInfo().abilityName_);
    ASSERT_EQ(windowMetaInfo1.parentWindowId, sceneSession->GetWindowId());
    ASSERT_EQ(windowMetaInfo1.surfaceNodeId, 0);
    ASSERT_EQ(windowMetaInfo1.leashWinSurfaceNodeId, 0);
    ASSERT_EQ(windowMetaInfo1.isPrivacyMode, true);
}

/**
 * @tc.name: GetWindowMetaInfoForWindowInfo02
 * @tc.desc: GetWindowMetaInfoForWindowInfo Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, GetWindowMetaInfoForWindowInfo02, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.isSystem_ = false;
    sessionInfo.bundleName_ = "bundleName";
    sessionInfo.abilityName_ = "abilityName";
    sessionInfo.windowType_ = static_cast<uint32_t>(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    sptr<SceneSession> sceneSession = sptr<SceneSession>::MakeSptr(sessionInfo, nullptr);
    WindowMetaInfo windowMetaInfo = sceneSession->GetWindowMetaInfoForWindowInfo();
    ASSERT_EQ(windowMetaInfo.isTouchable, true);
    auto screenId = 0;
    ScreenSessionConfig config;
    sptr<ScreenSession> screenSession =
        sptr<ScreenSession>::MakeSptr(config, ScreenSessionReason::CREATE_SESSION_FOR_CLIENT);
    ASSERT_NE(screenSession, nullptr);
    sptr<DisplayInfo> displayInfo = new(std::nothrow) DisplayInfo();
    ASSERT_NE(displayInfo, nullptr);
    displayInfo->SetScreenId(screenId);
    displayInfo->SetDisplayId(screenId);
    sceneSession->GetSessionProperty()->SetDisplayId(screenId);
    screenSession->SetTouchEnabledFromJs(false);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    windowMetaInfo = sceneSession->GetWindowMetaInfoForWindowInfo();
    ASSERT_EQ(windowMetaInfo.isTouchable, false);
    screenSession->SetTouchEnabledFromJs(true);
    sceneSession->SetSystemTouchable(false);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    windowMetaInfo = sceneSession->GetWindowMetaInfoForWindowInfo();
    ASSERT_EQ(windowMetaInfo.isTouchable, false);
    sceneSession->SetSystemTouchable(true);
    sceneSession->SetForegroundInteractiveStatus(false);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    windowMetaInfo = sceneSession->GetWindowMetaInfoForWindowInfo();
    ASSERT_EQ(windowMetaInfo.isTouchable, false);
    sceneSession->SetForegroundInteractiveStatus(true);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    windowMetaInfo = sceneSession->GetWindowMetaInfoForWindowInfo();
    ASSERT_EQ(windowMetaInfo.isTouchable, true);
    screenSession->SetTouchEnabledFromJs(false);
    sceneSession->SetSystemTouchable(false);
    sceneSession->SetForegroundInteractiveStatus(false);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.insert(std::make_pair(screenId, screenSession));
    windowMetaInfo = sceneSession->GetWindowMetaInfoForWindowInfo();
    ASSERT_EQ(windowMetaInfo.isTouchable, false);
    ScreenSessionManagerClient::GetInstance().screenSessionMap_.clear();
}

/**
 * @tc.name: GetWantSafely01
 * @tc.desc: GetWantSafely Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, GetWantSafely01, TestSize.Level1)
{
    SessionInfo sessionInfo;
    ASSERT_EQ(nullptr, sessionInfo.want);
    EXPECT_EQ(sessionInfo.GetWantSafely().GetBundle(), "");
}

/**
 * @tc.name: SetWantSafely01
 * @tc.desc: SetWantSafely Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetWantSafely01, TestSize.Level1)
{
    SessionInfo sessionInfo;
    AAFwk::Want wantObj;
    wantObj.SetBundle("SetWantSafelyTest");
    sessionInfo.SetWantSafely(wantObj);
    ASSERT_NE(nullptr, sessionInfo.want);
    EXPECT_EQ(sessionInfo.GetWantSafely().GetBundle(), "SetWantSafelyTest");
}

/**
 * @tc.name: IsNeedReportTimeout
 * @tc.desc: Case of non-specific window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, IsNeedReportTimeout_NonSpecific_Window, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "IsNeedReportTimeout_NonSpecific_Window";
    sessionInfo.bundleName_ = "IsNeedReportTimeout_NonSpecific_Window";

    sptr<Session> session = sptr<Session>::MakeSptr(sessionInfo);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->isSystemCalling_ = true;
    property->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    EXPECT_EQ(WindowType::APP_MAIN_WINDOW_BASE, property->GetWindowType());

    session->SetSessionProperty(property);
    EXPECT_EQ(WindowType::APP_MAIN_WINDOW_BASE, session->GetWindowType());
    EXPECT_EQ(false, session->IsNeedReportTimeout());

    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    EXPECT_EQ(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT, property->GetWindowType());

    session->SetSessionProperty(property);
    EXPECT_EQ(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT, session->GetWindowType());
    EXPECT_EQ(false, session->IsNeedReportTimeout());
}

/**
 * @tc.name: IsNeedReportTimeout
 * @tc.desc: Case of specific window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, IsNeedReportTimeout_specific_window, TestSize.Level1)
{
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "IsNeedReportTimeout_specific_window";
    sessionInfo.bundleName_ = "IsNeedReportTimeout_specific_window";

    sptr<Session> session = sptr<Session>::MakeSptr(sessionInfo);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->isSystemCalling_ = true;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    EXPECT_EQ(WindowType::WINDOW_TYPE_APP_SUB_WINDOW, property->GetWindowType());

    session->SetSessionProperty(property);
    EXPECT_EQ(WindowType::WINDOW_TYPE_APP_SUB_WINDOW, session->GetWindowType());
    EXPECT_EQ(true, session->IsNeedReportTimeout());
}

/**
 * @tc.name: PostSpecificSessionLifeCycleTimeoutTask
 * @tc.desc: Test Case about non specific window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, ReportWindowTimeout_NonSpecificWindow, TestSize.Level1)
{
    LOG_SetCallback(SessionTest4LogCallBack);
    g_logMsg.clear();
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "ReportWindowTimeout_NonSpecificWindow";
    sessionInfo.bundleName_ = "ReportWindowTimeout_NonSpecificWindow";

    sptr<Session> session = sptr<Session>::MakeSptr(sessionInfo);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->isSystemCalling_ = true;
    property->SetWindowType(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT);
    session->SetSessionProperty(property);
    EXPECT_EQ(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT, property->GetWindowType());
    EXPECT_EQ(WindowType::WINDOW_TYPE_INPUT_METHOD_FLOAT, session->GetWindowType());
    session->PostSpecificSessionLifeCycleTimeoutTask(ATTACH_EVENT_NAME);
    EXPECT_EQ(false, session->IsNeedReportTimeout());
}

/**
 * @tc.name: PostSpecificSessionLifeCycleTimeoutTask
 * @tc.desc: Test Case about non specific window
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, ReportWindowTimeout_SpecificWindow, TestSize.Level1)
{
    LOG_SetCallback(SessionTest4LogCallBack);
    g_logMsg.clear();
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "ReportWindowTimeout_SpecificWindow";
    sessionInfo.bundleName_ = "ReportWindowTimeout_SpecificWindow";

    sptr<Session> session = sptr<Session>::MakeSptr(sessionInfo);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->isSystemCalling_ = true;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    session->SetSessionProperty(property);
    EXPECT_EQ(WindowType::WINDOW_TYPE_APP_SUB_WINDOW, property->GetWindowType());
    EXPECT_EQ(WindowType::WINDOW_TYPE_APP_SUB_WINDOW, session->GetWindowType());
    session->PostSpecificSessionLifeCycleTimeoutTask(ATTACH_EVENT_NAME);
    EXPECT_TRUE(g_logMsg.find("not specific window") == std::string::npos);
}

/**
 * @tc.name: PostSpecificSessionLifeCycleTimeoutTask
 * @tc.desc: Test Case about handler is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, ReportWindowTimeout_Handler_NOT_NULL, TestSize.Level1)
{
    LOG_SetCallback(SessionTest4LogCallBack);
    g_logMsg.clear();
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "ReportWindowTimeout_Handler_NOT_NULL";
    sessionInfo.bundleName_ = "ReportWindowTimeout_Handler_NOT_NULL";

    sptr<Session> session = sptr<Session>::MakeSptr(sessionInfo);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->isSystemCalling_ = true;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    session->SetSessionProperty(property);
    EXPECT_EQ(WindowType::WINDOW_TYPE_APP_SUB_WINDOW, property->GetWindowType());
    EXPECT_EQ(WindowType::WINDOW_TYPE_APP_SUB_WINDOW, session->GetWindowType());
    session->PostSpecificSessionLifeCycleTimeoutTask(ATTACH_EVENT_NAME);

    sptr<SceneSessionManager> sceneSessionManager = sptr<SceneSessionManager>::MakeSptr();
    session_->SetEventHandler(sceneSessionManager->taskScheduler_->GetEventHandler(),
                              sceneSessionManager->eventHandler_);
    EXPECT_TRUE(g_logMsg.find("not specific window") == std::string::npos);
    EXPECT_TRUE(g_logMsg.find("handler is null") == std::string::npos);
}

/**
 * @tc.name: PostSpecificSessionLifeCycleTimeoutTask
 * @tc.desc: Test Case about window animation duration
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, ReportWindowTimeout_WindowAnimationDuration, TestSize.Level1)
{
    LOG_SetCallback(SessionTest4LogCallBack);
    g_logMsg.clear();
    SessionInfo sessionInfo;
    sessionInfo.abilityName_ = "ReportWindowTimeout_WindowAnimationDuration";
    sessionInfo.bundleName_ = "ReportWindowTimeout_WindowAnimationDuration";

    sptr<Session> session = sptr<Session>::MakeSptr(sessionInfo);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    property->isSystemCalling_ = true;
    property->SetWindowType(WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    session->SetSessionProperty(property);
    EXPECT_EQ(WindowType::WINDOW_TYPE_APP_SUB_WINDOW, property->GetWindowType());
    EXPECT_EQ(WindowType::WINDOW_TYPE_APP_SUB_WINDOW, session->GetWindowType());
    session->PostSpecificSessionLifeCycleTimeoutTask(ATTACH_EVENT_NAME);
    sptr<SceneSessionManager> sceneSessionManager = sptr<SceneSessionManager>::MakeSptr();
    session_->SetEventHandler(sceneSessionManager->taskScheduler_->GetEventHandler(),
                              sceneSessionManager->eventHandler_);
    EXPECT_TRUE(g_logMsg.find("handler is null") == std::string::npos);

    session->SetWindowAnimationDuration(true);
    EXPECT_EQ(true, session->IsNeedReportTimeout());

    session->SetWindowAnimationDuration(false);
    EXPECT_TRUE(g_logMsg.find("window configured animation") == std::string::npos);
}

/**
 * @tc.name: NotifyAppForceLandscapeConfigUpdated
 * @tc.desc: check func NotifyAppForceLandscapeConfigUpdated
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, NotifyAppForceLandscapeConfigUpdated, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->state_ = SessionState::STATE_CONNECT;
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(nullptr, mockSessionStage);
    session_->sessionStage_ = mockSessionStage;
    EXPECT_EQ(WSError::WS_OK, session_->NotifyAppForceLandscapeConfigUpdated());
    session_->sessionStage_ = nullptr;
    EXPECT_EQ(WSError::WS_ERROR_NULLPTR, session_->NotifyAppForceLandscapeConfigUpdated());
}

/**
 * @tc.name: SetLifeCycleTaskRunning
 * @tc.desc: check func SetLifeCycleTaskRunning
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetLifeCycleTaskRunning, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    auto task = [](){};
    std::string name = "testTask";
    sptr<Session::SessionLifeCycleTask> lifeCycleTask =
        sptr<Session::SessionLifeCycleTask>::MakeSptr(std::move(task), name, LifeCycleTaskType::STOP);

    bool ret = session_->SetLifeCycleTaskRunning(lifeCycleTask);

    EXPECT_TRUE(lifeCycleTask->running);
    EXPECT_TRUE(ret);

    ret = session_->SetLifeCycleTaskRunning(lifeCycleTask);
    EXPECT_FALSE(ret);

    sptr<Session::SessionLifeCycleTask> lifeCycleNullTask = nullptr;
    ret = session_->SetLifeCycleTaskRunning(lifeCycleNullTask);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name: SetHidingStartingWindow
 * @tc.desc: check func SetHidingStartingWindow
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetHidingStartingWindow, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->SetLeashWinSurfaceNode(nullptr);
    session_->SetHidingStartingWindow(false);
    EXPECT_TRUE(session_->GetHidingStartingWindow() == false);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session_->SetLeashWinSurfaceNode(surfaceNode);
    session_->SetHidingStartingWindow(true);
    EXPECT_TRUE(session_->GetHidingStartingWindow());
}

/**
 * @tc.name: SetLeashWindowAlpha
 * @tc.desc: check func SetLeashWindowAlpha
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetLeashWindowAlpha, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    session_->SetLeashWinSurfaceNode(nullptr);
    auto ret = session_->SetLeashWindowAlpha(false);
    EXPECT_EQ(ret, WSError::WS_ERROR_NULLPTR);

    struct RSSurfaceNodeConfig config;
    std::shared_ptr<RSSurfaceNode> surfaceNode = RSSurfaceNode::Create(config);
    session_->SetLeashWinSurfaceNode(surfaceNode);
    ret = session_->SetLeashWindowAlpha(true);
    EXPECT_EQ(ret, WSError::WS_OK);
}

/**
 * @tc.name: SetAndGetGlobalDisplayRect
 * @tc.desc: Verify that setting and getting global display rect works as expected
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetAndGetGlobalDisplayRect, TestSize.Level1)
{
    WSRect rect = { 10, 20, 200, 100 };
    session_->SetGlobalDisplayRect(rect);
    WSRect result = session_->GetGlobalDisplayRect();
    EXPECT_EQ(result, rect);
}

/**
 * @tc.name: TestUpdateGlobalDisplayRect
 * @tc.desc: Verify that updating global display rect works as expected
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, TestUpdateGlobalDisplayRect, TestSize.Level1)
{
    WSRect rect = { 10, 20, 200, 100 };
    session_->SetGlobalDisplayRect(rect);
    session_->globalDisplayRectSizeChangeReason_ = SizeChangeReason::RESIZE;

    auto ret = session_->UpdateGlobalDisplayRect(rect, SizeChangeReason::RESIZE);
    EXPECT_EQ(ret, WSError::WS_DO_NOTHING);
    EXPECT_EQ(session_->GetGlobalDisplayRect(), rect);
    EXPECT_EQ(session_->globalDisplayRectSizeChangeReason_, SizeChangeReason::RESIZE);

    ret = session_->UpdateGlobalDisplayRect(rect, SizeChangeReason::MOVE);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_EQ(session_->GetGlobalDisplayRect(), rect);
    EXPECT_EQ(session_->globalDisplayRectSizeChangeReason_, SizeChangeReason::MOVE);

    WSRect updated = { 30, 40, 200, 100 };
    ret = session_->UpdateGlobalDisplayRect(updated, SizeChangeReason::MOVE);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_EQ(session_->GetGlobalDisplayRect(), updated);
    EXPECT_EQ(session_->globalDisplayRectSizeChangeReason_, SizeChangeReason::MOVE);

    updated = { 0, 0, 200, 100 };
    ret = session_->UpdateGlobalDisplayRect(updated, SizeChangeReason::DRAG);
    EXPECT_EQ(ret, WSError::WS_OK);
    EXPECT_EQ(session_->GetGlobalDisplayRect(), updated);
    EXPECT_EQ(session_->globalDisplayRectSizeChangeReason_, SizeChangeReason::DRAG);
}

/**
 * @tc.name: TestNotifyClientToUpdateGlobalDisplayRect
 * @tc.desc: Verify that notifying client to update global display rect works as expected
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, TestNotifyClientToUpdateGlobalDisplayRect, TestSize.Level1)
{
    WSRect rect = { 10, 20, 200, 100 };
    auto originalState = session_->state_.load();
    auto originalSessionStage = session_->sessionStage_;

    session_->sessionStage_ = nullptr;
    auto result = session_->NotifyClientToUpdateGlobalDisplayRect(rect, SizeChangeReason::UNDEFINED);
    EXPECT_EQ(result, WSError::WS_DO_NOTHING);

    auto mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    session_->sessionStage_ = mockSessionStage;
    session_->state_ = SessionState::STATE_BACKGROUND;
    result = session_->NotifyClientToUpdateGlobalDisplayRect(rect, SizeChangeReason::UNDEFINED);
    EXPECT_EQ(result, WSError::WS_DO_NOTHING);

    session_->state_ = SessionState::STATE_FOREGROUND;
    EXPECT_CALL(*mockSessionStage, UpdateGlobalDisplayRectFromServer(rect, SizeChangeReason::UNDEFINED))
        .WillOnce(Return(WSError::WS_OK));
    result = session_->NotifyClientToUpdateGlobalDisplayRect(rect, SizeChangeReason::UNDEFINED);
    EXPECT_EQ(result, WSError::WS_OK);
    session_->state_ = originalState;
    session_->sessionStage_ = originalSessionStage;
}

/**
 * @tc.name: TestGetSessionScreenRelativeRect_001
 * @tc.desc: get relative rect when reason is not drag move
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, TestGetSessionScreenRelativeRect_001, TestSize.Level1)
{
    session_->UpdateSizeChangeReason(SizeChangeReason::RESIZE);
    WSRect expectedRect = { 0, 0, 100, 100};
    session_->SetSessionRect(expectedRect);

    WSRect result = session_->GetSessionScreenRelativeRect();
    EXPECT_EQ(result, expectedRect);
}

class LayoutControllerMocker : public LayoutController {
public:
    explicit LayoutControllerMocker(const sptr<WindowSessionProperty>& property) : LayoutController(property) {};
    ~LayoutControllerMocker() {};
    MOCK_METHOD2(ConvertGlobalRectToRelative, WSRect(const WSRect& globalRect, DisplayId targetDisplayId));
};

/**
 * @tc.name: TestGetSessionScreenRelativeRect_001
 * @tc.desc: get relative rect when reason is not drag move
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, TestGetSessionScreenRelativeRect_002, TestSize.Level1)
{
    SessionInfo sessionInfo = { "CreateTestBundle", "CreateTestModule", "CreateTestAbility" };
    sptr<SessionMocker> session = sptr<SessionMocker>::MakeSptr(sessionInfo);
    sptr<WindowSessionProperty> property = sptr<WindowSessionProperty>::MakeSptr();
    sptr<LayoutControllerMocker> layoutController = sptr<LayoutControllerMocker>::MakeSptr(property);

    session->SetMockLayoutController(layoutController);
    session_->UpdateSizeChangeReason(SizeChangeReason::DRAG_MOVE);
    WSRect expectedRect = { 0, 0, 50, 50};
    WSRect winRect = { 0, 0, 50, 50};
    session->SetSessionRect(winRect);

    EXPECT_CALL(*layoutController, ConvertGlobalRectToRelative(_, _)).Times(1).WillOnce(Return(expectedRect));
    WSRect result = session_->GetSessionScreenRelativeRect();
    EXPECT_EQ(result, expectedRect);
}

/**
 * @tc.name: HasParentSessionWithToken
 * @tc.desc: get relative rect when reason is not drag move
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, HasParentSessionWithToken, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "HasParentSessionWithToken";
    info.bundleName_ = "HasParentSessionWithToken";
    sptr<Session> session = sptr<Session>::MakeSptr(info);

    sptr<IRemoteObject> token = sptr<MockIRemoteObject>::MakeSptr();
    bool ret = session->HasParentSessionWithToken(token);
    EXPECT_EQ(ret, false);

    SessionInfo parentSessionInfo;
    parentSessionInfo.abilityName_ = "parentSession";
    parentSessionInfo.bundleName_ = "parentSession";
    sptr<Session> parentSession = sptr<Session>::MakeSptr(parentSessionInfo);
    session->SetParentSession(parentSession);

    ret = session->HasParentSessionWithToken(token);
    EXPECT_EQ(ret, false);

    parentSession->SetAbilityToken(token);
    ret = session->HasParentSessionWithToken(token);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: SetIsShowDecorInFreeMultiWindow 01
 * @tc.desc: Test Case SetIsShowDecorInFreeMultiWindow 01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetIsShowDecorInFreeMultiWindow01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool isShow = true;
    session_->state_ = SessionState::STATE_DISCONNECT;
    WSError result = session_->SetIsShowDecorInFreeMultiWindow(isShow);
    EXPECT_EQ(result, WSError::WS_ERROR_INVALID_SESSION);

    session_->state_ = SessionState::STATE_CONNECT;
    session_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    WSError result02 = session_->SetIsShowDecorInFreeMultiWindow(isShow);
    EXPECT_EQ(result02, WSError::WS_ERROR_NULLPTR);
}

/**
 * @tc.name: UpdateRect
 * @tc.desc: Test Case UpdateRect
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, UpdateRect, TestSize.Level1)
{
    WSRect rect;
    SizeChangeReason reason = SizeChangeReason::SCENE_WITH_ANIMATION;
    std::shared_ptr<RSTransaction> rsTransaction = std::make_shared<RSTransaction>();
    std::shared_ptr<AvoidArea> avoidArea = std::make_shared<AvoidArea>();
    session_->state_ = SessionState::STATE_CONNECT;
    sptr<SessionStageMocker> mockSessionStage = sptr<SessionStageMocker>::MakeSptr();
    ASSERT_NE(nullptr, mockSessionStage);
    session_->sessionStage_ = mockSessionStage;
    WSError res = session_->UpdateRect(rect, reason, "UpdateRect", rsTransaction);
    ASSERT_EQ(WSError::WS_OK, res);
}

/**
 * @tc.name: SetIsShowDecorInFreeMultiWindow 02
 * @tc.desc: Test Case SetIsShowDecorInFreeMultiWindow 02
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetIsShowDecorInFreeMultiWindow02, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    bool isShow = true;
    session_->state_ = SessionState::STATE_CONNECT;
    session_->sessionStage_ = sptr<SessionStageMocker>::MakeSptr();
    session_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    WSError result = session_->SetIsShowDecorInFreeMultiWindow(isShow);
    EXPECT_EQ(result, WSError::WS_OK);

    session_->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_END);
    WSError result02 = session_->SetIsShowDecorInFreeMultiWindow(isShow);
    EXPECT_EQ(result02, WSError::WS_OK);
}

/**
 * @tc.name: UpdateSessionOutline
 * @tc.desc: Test func UpdateSessionOutline
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, UpdateSessionOutline, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);

    OutlineStyleParams defaultParams;
    bool enabled = false;
    session_->UpdateSessionOutline(enabled, defaultParams);
    EXPECT_EQ(session_->isOutlineEnabled_, false);

    enabled = true;
    session_->UpdateSessionOutline(enabled, defaultParams);
    EXPECT_EQ(session_->isOutlineEnabled_, true);

    defaultParams.outlineColor_ = 0x00ffffff; // 0x00ffffff: color has no alpha byte.
    session_->UpdateSessionOutline(enabled, defaultParams);
    EXPECT_EQ(session_->outlineStyleParams_.outlineColor_, 0x00ffffff); // 0x00ffffff: color has no alpha byte.
}

/**
 * @tc.name: SetOutlineParamsChangeCallback
 * @tc.desc: Test func SetOutlineParamsChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, SetOutlineParamsChangeCallback, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetOutlineParamsChangeCallback(nullptr);
    EXPECT_EQ(session_->outlineParamsChangeCallback_, nullptr);

    auto func = [](bool enabled, const OutlineStyleParams& outlineStyleParams) {};
    session_->SetOutlineParamsChangeCallback(std::move(func));
    usleep(waitSyncInNs_);
    EXPECT_NE(session_->outlineParamsChangeCallback_, nullptr);
}

/**
 * @tc.name: UpdateSessionOutline01
 * @tc.desc: Test func UpdateSessionOutline
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, UpdateSessionOutline01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    auto func = [](bool enabled, const OutlineStyleParams& outlineStyleParams) {};
    session_->SetOutlineParamsChangeCallback(std::move(func));
    usleep(waitSyncInNs_);
    OutlineStyleParams defaultParams;
    defaultParams.outlineColor_ = 0x000000ff; // 0x000000ff: color blue byte.
    bool enabled = false;
    session_->UpdateSessionOutline(enabled, defaultParams);
    EXPECT_EQ(session_->outlineStyleParams_.outlineColor_, 0x000000ff); // 0x000000ff: color blue byte.
}

/**
 * @tc.name: CheckEmptyKeyboardAvoidAreaIfNeeded 01
 * @tc.desc: Test Case CheckEmptyKeyboardAvoidAreaIfNeeded 01
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, CheckEmptyKeyboardAvoidAreaIfNeeded01, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    DisplayId displayId = 0;
    session_->property_->SetDisplayId(displayId);
    bool result = session_->CheckEmptyKeyboardAvoidAreaIfNeeded();
    EXPECT_EQ(result, false);

    displayId = 100;
    session_->property_->SetDisplayId(displayId);
    result = session_->CheckEmptyKeyboardAvoidAreaIfNeeded();
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: PrelaunchCheck
 * @tc.desc: Test Case PrelaunchCheck
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, PrelaunchCheck, TestSize.Level1)
{
    ASSERT_NE(session_, nullptr);
    session_->SetPrelaunch();
    bool result = session_->IsPrelaunch();
    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetIsMidScene_SubSession
 * @tc.desc: GetIsMidScene_SubSession Test
 * @tc.type: FUNC
 */
HWTEST_F(WindowSessionTest4, GetIsMidScene_SubSession, TestSize.Level1)
{
    SessionInfo info;
    info.abilityName_ = "GetIsMidScene";
    info.moduleName_ = "GetIsMidScene";
    info.bundleName_ = "GetIsMidScene";
    sptr<Session> subSession = sptr<Session>::MakeSptr(info);
    ASSERT_NE(subSession, nullptr);
    subSession->SetIsMidScene(false);
    ASSERT_NE(session_, nullptr);
    subSession->SetParentSession(session_);

    session_->SetIsMidScene(false);
    sleep(1);
    bool isMidScene = false;
    auto result = subSession->GetIsMidScene(isMidScene);
    EXPECT_EQ(result, WSError::WS_OK);
    EXPECT_EQ(isMidScene, false);

    session_->SetIsMidScene(true);
    sleep(1);
    isMidScene = false;
    result = subSession->GetIsMidScene(isMidScene);
    EXPECT_EQ(result, WSError::WS_OK);
    EXPECT_EQ(isMidScene, true);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
