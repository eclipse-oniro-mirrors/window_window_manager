/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include <pixel_map.h>

#include "display_manager_adapter.h"
#include "mission_listener_stub.h"
#include "scene_board_judgement.h"
#include "scene_session_manager.h"
#include "session/host/include/main_session.h"
#include "session/host/include/scene_session.h"
#include "session_listener_controller.h"
#include "singleton_container.h"
#include "window_manager_hilog.h"
#include "zidl/session_lifecycle_listener_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Media;
namespace OHOS {
namespace Rosen {
namespace {
    std::string g_logMsg;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_logMsg = msg;
    }
}
static const std::string LISTENER_CONTROLLER_TEST_THREAD = "OS_ListenerControllerTest";

class MyMissionListener : public AAFwk::MissionListenerStub {
public:
    MyMissionListener() = default;
    ~MyMissionListener() = default;

    void OnMissionCreated(int32_t missionId) override
    {
        isMissionCreated_ = true;
    }

    void OnMissionDestroyed(int32_t missionId) override
    {
        isMissionDestroyed_ = true;
    }

    void OnMissionMovedToBackground(int32_t missionId) override
    {
        isMissionBackground_ = true;
    }

    void OnMissionSnapshotChanged(int32_t missionId) override
    {
        isMissionSnapshotChanged_ = true;
    }

    void OnMissionMovedToFront(int32_t missionId) override
    {
        isMissionMovedToFront_ = true;
    }

#ifdef SUPPORT_GRAPHICS
    void OnMissionIconUpdated(int32_t missionId, const std::shared_ptr<OHOS::Media::PixelMap>& icon) override
    {
        isMissionIconUpdated_ = true;
    }
#endif

    void OnMissionClosed(int32_t missionId) override
    {
        isMissionClosed_ = true;
    }

    void OnMissionLabelUpdated(int32_t missionId) override
    {
        isMissionLabelUpdated_ = true;
    }

    bool IsMissionCreated() const
    {
        return isMissionCreated_;
    }

    bool IsMissionDestroyed() const
    {
        return isMissionDestroyed_;
    }
    
    bool IsMissionBackground() const
    {
        return isMissionBackground_;
    }

    bool IsMissionSnapshotChanged() const
    {
        return isMissionSnapshotChanged_;
    }

    bool IsMissionMovedToFront() const
    {
        return isMissionMovedToFront_;
    }

    bool IsMissionIconUpdated() const
    {
        return isMissionIconUpdated_;
    }

    bool IsMissionClosed() const
    {
        return isMissionClosed_;
    }

    bool IsMissionLabelUpdated() const
    {
        return isMissionLabelUpdated_;
    }

private:
    bool isMissionCreated_ = false;
    bool isMissionDestroyed_ = false;
    bool isMissionSnapshotChanged_ = false;
    bool isMissionMovedToFront_ = false;
    bool isMissionIconUpdated_ = false;
    bool isMissionClosed_ = false;
    bool isMissionLabelUpdated_ = false;
    bool isMissionBackground_ = false;
};

class SessionListenerControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<TaskScheduler> taskScheduler_ = std::make_shared<TaskScheduler>(LISTENER_CONTROLLER_TEST_THREAD);
    std::shared_ptr<SessionListenerController> slController;
    static sptr<SceneSessionManager> ssm_;

private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};

class MySessionLifecycleListener : public Rosen::SessionLifecycleListenerStub {
public:
    MySessionLifecycleListener() = default;
    ~MySessionLifecycleListener() override = default;
    void OnLifecycleEvent(SessionLifecycleEvent event, const LifecycleEventPayload& payload) override
    {
        event_ = event;
        payload_ = payload;
    }

private:
    ISessionLifecycleListener::SessionLifecycleEvent event_;
    ISessionLifecycleListener::LifecycleEventPayload payload_;
};

sptr<SceneSessionManager> SessionListenerControllerTest::ssm_ = nullptr;

void SessionListenerControllerTest::SetUpTestCase()
{
    ssm_ = &SceneSessionManager::GetInstance();
}

void SessionListenerControllerTest::TearDownTestCase()
{
    ssm_ = nullptr;
}

void SessionListenerControllerTest::SetUp()
{
    slController = std::make_shared<SessionListenerController>(taskScheduler_);
}

void SessionListenerControllerTest::TearDown()
{
    usleep(WAIT_SYNC_IN_NS);
}

namespace {
/**
 * @tc.name: AddSessionListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, AddSessionListener, TestSize.Level1)
{
    sptr<ISessionListener> listener;
    ASSERT_EQ(listener, nullptr);
    WSError res = slController->AddSessionListener(listener);
    ASSERT_EQ(res, WSError::WS_ERROR_INVALID_PARAM);

    listener = sptr<MyMissionListener>::MakeSptr();
    EXPECT_NE(nullptr, listener);
    res = slController->AddSessionListener(listener);
    EXPECT_EQ(WSError::WS_OK, res);

    auto iSession = listener;
    WSError newRes = slController->AddSessionListener(iSession);
    EXPECT_EQ(WSError::WS_OK, newRes);
}

/**
 * @tc.name: DelSessionListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, DelSessionListener, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    sptr<ISessionListener> listener;
    ASSERT_EQ(listener, nullptr);
    slController->DelSessionListener(listener);
    EXPECT_TRUE(g_logMsg.find("listener is invalid") != std::string::npos);
    int32_t persistentId = 1;
    slController->NotifySessionLabelUpdated(persistentId);
    EXPECT_TRUE(g_logMsg.find("1") != std::string::npos);

    listener = sptr<MyMissionListener>::MakeSptr();
    slController->DelSessionListener(listener);
    EXPECT_NE(nullptr, listener);
    EXPECT_FALSE(g_logMsg.find("success") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: NotifySessionCreated
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionCreated, TestSize.Level1)
{
    int32_t persistentId = -1;
    slController->NotifySessionCreated(persistentId);

    persistentId = 1;
    slController->NotifySessionCreated(persistentId);
    ASSERT_EQ(persistentId, 1);

    slController->NotifySessionCreated(persistentId);
    EXPECT_EQ(persistentId, 1);
}

/**
 * @tc.name: NotifySessionBackground
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionBackground, TestSize.Level1)
{
    int32_t persistentId = -1;
    ASSERT_NE(slController, nullptr);
    slController->NotifySessionBackground(persistentId);

    persistentId = 1;
    slController->NotifySessionBackground(persistentId);
}

/**
 * @tc.name: NotifySessionDestroyed
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionDestroyed, TestSize.Level1)
{
    int32_t persistentId = -1;
    slController->NotifySessionDestroyed(persistentId);

    persistentId = 1;
    slController->NotifySessionDestroyed(persistentId);
    ASSERT_EQ(persistentId, 1);

    slController->NotifySessionDestroyed(persistentId);
    EXPECT_EQ(1, persistentId);
}

/**
 * @tc.name: HandleUnInstallApp1
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, HandleUnInstallApp1, TestSize.Level1)
{
    std::list<int32_t> sessions;
    slController->HandleUnInstallApp(sessions);
    EXPECT_EQ(0, sessions.size());
}

/**
 * @tc.name: HandleUnInstallApp2
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, HandleUnInstallApp2, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    std::list<int32_t> sessions;
    sessions.push_front(1);
    slController->HandleUnInstallApp(sessions);
    EXPECT_NE(0, sessions.size());
    int32_t persistentId = 1;
    slController->NotifySessionLabelUpdated(persistentId);
    EXPECT_TRUE(g_logMsg.find("1") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: NotifySessionSnapshotChanged
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionSnapshotChanged, TestSize.Level1)
{
    int32_t persistentId = -1;
    slController->NotifySessionSnapshotChanged(persistentId);

    persistentId = 1;
    slController->NotifySessionSnapshotChanged(persistentId);
    ASSERT_EQ(persistentId, 1);
}

/**
 * @tc.name: NotifySessionMovedToFront
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionMovedToFront, TestSize.Level1)
{
    int32_t persistentId = -1;
    slController->NotifySessionMovedToFront(persistentId);

    persistentId = 1;
    slController->NotifySessionMovedToFront(persistentId);
    ASSERT_EQ(persistentId, 1);
}

/**
 * @tc.name: NotifySessionFocused
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionFocused, TestSize.Level1)
{
    int32_t persistentId = -1;
    slController->NotifySessionFocused(persistentId);

    persistentId = 1;
    slController->NotifySessionFocused(persistentId);
    ASSERT_EQ(persistentId, 1);
}

/**
 * @tc.name: NotifySessionUnfocused
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionUnfocused, TestSize.Level1)
{
    int32_t persistentId = -1;
    slController->NotifySessionUnfocused(persistentId);

    persistentId = 1;
    slController->NotifySessionUnfocused(persistentId);
    ASSERT_EQ(persistentId, 1);
}

/**
 * @tc.name: NotifySessionClosed
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionClosed, TestSize.Level1)
{
    g_logMsg.clear();
    LOG_SetCallback(MyLogCallback);
    SessionInfo info;
    info.bundleName_ = "bundleName";
    info.moduleName_ = "moduleName";
    info.abilityName_ = "abilityName";
    info.appIndex_ = 0;
    info.persistentId_ = -1;
    slController->NotifySessionClosed(info);
    EXPECT_FALSE(g_logMsg.find("-1") != std::string::npos);

    info.persistentId_ = 1;
    slController->NotifySessionClosed(info);
    EXPECT_TRUE(g_logMsg.find("1") != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: NotifySessionLabelUpdated
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionLabelUpdated, TestSize.Level1)
{
    int32_t persistentId = -1;
    slController->NotifySessionLabelUpdated(persistentId);

    persistentId = 1;
    slController->NotifySessionLabelUpdated(persistentId);
    ASSERT_EQ(persistentId, 1);
}

/**
 * @tc.name: OnListenerDied
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, OnListenerDied, TestSize.Level1)
{
    sptr<IRemoteObject> remote;
    ASSERT_NE(nullptr, slController);
    slController->OnListenerDied(remote);
    EXPECT_EQ(nullptr, remote);

    if (SingletonContainer::Get<ScreenManagerAdapter>().InitDMSProxy()) {
        if (SceneBoardJudgement::IsSceneBoardEnabled()) {
            remote = SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_->AsObject();
        } else {
            remote = SingletonContainer::Get<ScreenManagerAdapter>().displayManagerServiceProxy_->AsObject();
        }
        slController->OnListenerDied(remote);
        EXPECT_NE(nullptr, remote);
    }
}

/**
 * @tc.name: NotifySessionIconChanged
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionIconChanged, TestSize.Level1)
{
#ifndef SUPPORT_GRAPHICS
#define SUPPORT_GRAPHICS
    int32_t persistentId = -1;

    int32_t pixelMapWidth = 4;
    int32_t pixelMapHeight = 3;
    std::unique_ptr<OHOS::Media::PixelMap> pixelMap = std::make_unique<OHOS::Media::PixelMap>();
    OHOS::Media::ImageInfo info;
    info.size.width = pixelMapWidth;
    info.size.height = pixelMapHeight;
    info.pixelFormat = OHOS::Media::PixelFormat::RGB_888;
    pixelMap->SetImageInfo(info);
    std::shared_ptr<OHOS::Media::PixelMap> icon = std::move(pixelMap);
    slController->NotifySessionIconChanged(persistentId, icon);
    sptr<MyMissionListener> myListener = sptr<MyMissionListener>::MakeSptr();
    EXPECT_NE(myListener, nullptr);
    bool res = myListener->IsMissionIconUpdated();
    EXPECT_EQ(res, false);

    persistentId = 1;
    sptr<ISessionListener> listener = sptr<MyMissionListener>::MakeSptr();
    EXPECT_NE(listener, nullptr);
    slController->sessionListeners_.push_back(listener);
    slController->NotifySessionIconChanged(persistentId, icon);
    res = myListener->IsMissionIconUpdated();
    EXPECT_EQ(res, true);
    slController->sessionListeners_.clear();
#endif
}

/**
 * @tc.name: ListenerDeathRecipient
 * @tc.desc: ListenerDeathRecipient class test
 * @tc.type: CLASS
 */
HWTEST_F(SessionListenerControllerTest, ListenerDeathRecipient, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TaskSchedulerText: task_scheduler_test001 start";
    EXPECT_EQ(nullptr, slController->listenerDeathRecipient_);
    sptr<ISessionListener> listener = sptr<MyMissionListener>::MakeSptr();
    slController->AddSessionListener(listener);
    EXPECT_NE(nullptr, slController->listenerDeathRecipient_);

    if (SingletonContainer::Get<ScreenManagerAdapter>().InitDMSProxy()) {
        sptr<IRemoteObject> remote;
        if (SceneBoardJudgement::IsSceneBoardEnabled()) {
            remote = SingletonContainer::Get<ScreenManagerAdapter>().screenSessionManagerServiceProxy_->AsObject();
        } else {
            remote = SingletonContainer::Get<ScreenManagerAdapter>().displayManagerServiceProxy_->AsObject();
        }
        slController->listenerDeathRecipient_->OnRemoteDied(remote);
        EXPECT_NE(nullptr, remote);
    }
    GTEST_LOG_(INFO) << "TaskSchedulerText: task_scheduler_test001 end";
}

/**
 * @tc.name: Register
 * @tc.desc: Register By Bundle
 * @tc.type: CLASS
 */
HWTEST_F(SessionListenerControllerTest, RegisterSessionLifecycleListenerByBundles, TestSize.Level1)
{
    std::vector<std::string> bundleNameList1 = { "bundle1", "bundle2" };
    WMError res = slController->RegisterSessionLifecycleListener(nullptr, bundleNameList1);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);

    sptr<ISessionLifecycleListener> listener = sptr<MySessionLifecycleListener>::MakeSptr();
    ASSERT_NE(listener, nullptr);
    res = slController->RegisterSessionLifecycleListener(listener, bundleNameList1);
    ASSERT_EQ(res, WMError::WM_OK);

    std::vector<std::string> bundleNameList2;
    res = slController->RegisterSessionLifecycleListener(listener, bundleNameList2);
    ASSERT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: Register
 * @tc.desc: Register By Id
 * @tc.type: CLASS
 */
HWTEST_F(SessionListenerControllerTest, RegisterSessionLifecycleListenerByIds, TestSize.Level1)
{
    std::vector<int32_t> persistentIdList1 = { 1, 2 };
    WMError res = slController->RegisterSessionLifecycleListener(nullptr, persistentIdList1);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);

    sptr<ISessionLifecycleListener> listener = sptr<MySessionLifecycleListener>::MakeSptr();
    ASSERT_NE(listener, nullptr);
    res = slController->RegisterSessionLifecycleListener(listener, persistentIdList1);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);

    std::vector<int32_t> persistentIdList2;
    res = slController->RegisterSessionLifecycleListener(listener, persistentIdList2);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);

    SessionInfo info;
    info.bundleName_ = "com.example.myapp";
    info.abilityName_ = "MainAbility";
    info.moduleName_ = "entry";
    info.persistentId_ = 101;
    info.appIndex_ = 0;
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sceneSession->property_->SetWindowType(WindowType::APP_MAIN_WINDOW_BASE);
    ssm_->sceneSessionMap_.insert({ 101, sceneSession });
    res = slController->RegisterSessionLifecycleListener(listener, persistentIdList1);
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: RegisterSessionLifecycleListenerByAppInstance
 * @tc.desc: Register session lifecycle listener by bundle+appIndex+appInstanceKey
 * @tc.type: CLASS
 */
HWTEST_F(SessionListenerControllerTest, RegisterSessionLifecycleListenerByAppInstance, TestSize.Level1)
{
    WMError res = slController->RegisterSessionLifecycleListener(nullptr, "com.example.myapp", 0, "");
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);

    sptr<ISessionLifecycleListener> listener = sptr<MySessionLifecycleListener>::MakeSptr();
    ASSERT_NE(listener, nullptr);

    res = slController->RegisterSessionLifecycleListener(listener, "", 0, "");
    EXPECT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);

    res = slController->RegisterSessionLifecycleListener(listener, "com.example.myapp", 0, "");
    EXPECT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: UnregisterSessionLifecycleListener
 * @tc.desc: UnregisterSessionLifecycleListener
 * @tc.type: CLASS
 */
HWTEST_F(SessionListenerControllerTest, UnregisterSessionLifecycleListener, TestSize.Level1)
{
    WMError res = slController->UnregisterSessionLifecycleListener(nullptr);
    ASSERT_EQ(res, WMError::WM_ERROR_INVALID_PARAM);

    sptr<ISessionLifecycleListener> listener = sptr<MySessionLifecycleListener>::MakeSptr();
    ASSERT_NE(listener, nullptr);
    res = slController->UnregisterSessionLifecycleListener(listener);
    ASSERT_EQ(res, WMError::WM_OK);
}

/**
 * @tc.name: NotifySessionLifecycleEvent01
 * @tc.desc: NotifySessionLifecycleEvent01
 * @tc.type: CLASS
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionLifecycleEvent01, Function | SmallTest | Level2)
{
    sptr<MySessionLifecycleListener> myListener = new MySessionLifecycleListener();
    sptr<ISessionLifecycleListener> listener = iface_cast<ISessionLifecycleListener>(myListener->AsObject());
    ASSERT_NE(listener, nullptr);
    std::vector<std::string> bundleNameList;
    slController->RegisterSessionLifecycleListener(listener, bundleNameList);

    SessionInfo info;
    info.bundleName_ = "com.example.myapp";
    info.abilityName_ = "MainAbility";
    info.moduleName_ = "entry";
    info.persistentId_ = 101;
    info.appIndex_ = 0;

    slController->NotifySessionLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::CREATED, info);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(myListener->event_, ISessionLifecycleListener::SessionLifecycleEvent::CREATED);

    slController->UnregisterSessionLifecycleListener(listener);
    bundleNameList.emplace_back("com.example.myapp");
    slController->RegisterSessionLifecycleListener(listener, bundleNameList);
    slController->NotifySessionLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::BACKGROUND, info);
    usleep(WAIT_SYNC_IN_NS);
    ASSERT_EQ(myListener->event_, ISessionLifecycleListener::SessionLifecycleEvent::BACKGROUND);
}

/**
 * @tc.name: NotifySessionLifecycleEvent02
 * @tc.desc: NotifySessionLifecycleEvent02
 * @tc.type: CLASS
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionLifecycleEvent02, Function | SmallTest | Level2)
{
    sptr<MySessionLifecycleListener> myListener = new MySessionLifecycleListener();
    sptr<ISessionLifecycleListener> listener = iface_cast<ISessionLifecycleListener>(myListener->AsObject());
    ASSERT_NE(listener, nullptr);
    std::vector<int32_t> persistentIdList = { 102 };
    SessionInfo info;
    info.bundleName_ = "com.example.myapp";
    info.abilityName_ = "MainAbility";
    info.moduleName_ = "entry";
    info.persistentId_ = 102;
    info.appIndex_ = 0;
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->sceneSessionMap_.insert({ 102, sceneSession });
    slController->RegisterSessionLifecycleListener(listener, persistentIdList);
    slController->NotifySessionLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::CREATED, info);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(myListener->event_, ISessionLifecycleListener::SessionLifecycleEvent::CREATED);
}

/**
 * @tc.name: NotifySessionLifecycleEvent03
 * @tc.desc: NotifySessionLifecycleEvent03
 * @tc.type: CLASS
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionLifecycleEvent03, Function | SmallTest | Level2)
{
    sptr<MySessionLifecycleListener> myListener = new MySessionLifecycleListener();
    sptr<ISessionLifecycleListener> listener = iface_cast<ISessionLifecycleListener>(myListener->AsObject());
    ASSERT_NE(listener, nullptr);
    std::vector<int32_t> persistentIdList = { 103 };
    SessionInfo info;
    info.bundleName_ = "com.example.myapp";
    info.abilityName_ = "MainAbility";
    info.moduleName_ = "entry";
    info.persistentId_ = 103;
    info.appIndex_ = 0;
    sptr<SceneSession> sceneSession = sptr<MainSession>::MakeSptr(info, nullptr);
    sceneSession->property_->SetWindowType(WindowType::WINDOW_TYPE_APP_MAIN_WINDOW);
    ssm_->sceneSessionMap_.insert({ 103, sceneSession });
    sceneSession->SetSessionState(SessionState::STATE_CONNECT);
    slController->RegisterSessionLifecycleListener(listener, persistentIdList);
    ssm_->RequestSceneSessionActivation(sceneSession, true);
    slController->NotifySessionLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::CREATED, info);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(myListener->event_, ISessionLifecycleListener::SessionLifecycleEvent::CREATED);

    sceneSession->SetSessionState(SessionState::STATE_ACTIVE);
    ssm_->RequestSceneSessionActivation(sceneSession, true);
    slController->NotifySessionLifecycleEvent(ISessionLifecycleListener::SessionLifecycleEvent::ACTIVE, info);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(myListener->event_, ISessionLifecycleListener::SessionLifecycleEvent::ACTIVE);
}

/**
 * @tc.name: NotifySessionTransferToTargetScreenEvent01_ByPersistentId
 * @tc.desc: NotifySessionTransferToTargetScreenEvent01_ByPersistentId
 * @tc.type: CLASS
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionTransferToTargetScreenEvent01_ByPersistentId, TestSize.Level1)
{
    sptr<MySessionLifecycleListener> myListener = new MySessionLifecycleListener();
    sptr<ISessionLifecycleListener> listener = iface_cast<ISessionLifecycleListener>(myListener->AsObject());
    ASSERT_NE(listener, nullptr);
    std::vector<int32_t> persistentIdList = { 111 };
    slController->RegisterSessionLifecycleListener(listener, persistentIdList);

    SessionInfo info;
    info.bundleName_ = "com.example.myapp";
    info.abilityName_ = "MainAbility";
    info.moduleName_ = "entry";
    info.persistentId_ = -1;
    info.appIndex_ = 0;
    uint32_t resultCode = 0;
    uint64_t fromScreenId = 0;
    uint64_t toScreenId = 1;

    slController->NotifySessionTransferToTargetScreenEvent(info, resultCode, fromScreenId, toScreenId);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(myListener->event_, ISessionLifecycleListener::SessionLifecycleEvent::CREATED);

    info.persistentId_ = 111;
    LifeCycleChangeReason reason = LifeCycleChangeReason::BACK_TO_DESKTOP;
    slController->NotifySessionTransferToTargetScreenEvent(info, resultCode, fromScreenId, toScreenId, reason);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(myListener->event_, ISessionLifecycleListener::SessionLifecycleEvent::TRANSFER_TO_TARGET_SCREEN);
    EXPECT_EQ(myListener->payload_.persistentId_, info.persistentId_);
    EXPECT_EQ(myListener->payload_.fromScreenId_, fromScreenId);
    EXPECT_EQ(myListener->payload_.toScreenId_, toScreenId);
    EXPECT_EQ(myListener->payload_.resultCode_, resultCode);
    EXPECT_EQ(myListener->payload_.lifeCycleChangeReason_, reason);
}

/**
 * @tc.name: NotifySessionTransferToTargetScreenEvent02_ByBundleName
 * @tc.desc: NotifySessionTransferToTargetScreenEvent02_ByBundleName
 * @tc.type: CLASS
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionTransferToTargetScreenEvent02_ByBundleName, TestSize.Level1)
{
    sptr<MySessionLifecycleListener> myListener = new MySessionLifecycleListener();
    sptr<ISessionLifecycleListener> listener = iface_cast<ISessionLifecycleListener>(myListener->AsObject());
    ASSERT_NE(listener, nullptr);
    std::vector<std::string> bundleNameList;
    bundleNameList.emplace_back("com.example.myapp");
    slController->RegisterSessionLifecycleListener(listener, bundleNameList);

    SessionInfo info;
    info.bundleName_ = "com.example.myapp";
    info.abilityName_ = "MainAbility";
    info.moduleName_ = "entry";
    info.persistentId_ = 11;
    info.appIndex_ = 0;
    uint32_t resultCode = 0;
    uint64_t fromScreenId = 0;
    uint64_t toScreenId = 1;

    slController->NotifySessionTransferToTargetScreenEvent(info, resultCode, fromScreenId, toScreenId);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(myListener->event_, ISessionLifecycleListener::SessionLifecycleEvent::TRANSFER_TO_TARGET_SCREEN);
    EXPECT_EQ(myListener->payload_.persistentId_, info.persistentId_);
    EXPECT_EQ(myListener->payload_.fromScreenId_, fromScreenId);
    EXPECT_EQ(myListener->payload_.toScreenId_, toScreenId);
    EXPECT_EQ(myListener->payload_.resultCode_, resultCode);
}

/**
 * @tc.name: NotifySessionTransferToTargetScreenEvent03_ByAll
 * @tc.desc: NotifySessionTransferToTargetScreenEvent03_ByAll
 * @tc.type: CLASS
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionTransferToTargetScreenEvent03_ByAll, TestSize.Level1)
{
    sptr<MySessionLifecycleListener> myListener = new MySessionLifecycleListener();
    sptr<ISessionLifecycleListener> listener = iface_cast<ISessionLifecycleListener>(myListener->AsObject());
    ASSERT_NE(listener, nullptr);
    std::vector<std::string> bundleNameList;
    slController->RegisterSessionLifecycleListener(listener, bundleNameList);

    SessionInfo info;
    info.bundleName_ = "com.example.myapp";
    info.abilityName_ = "MainAbility";
    info.moduleName_ = "entry";
    info.persistentId_ = 11;
    info.appIndex_ = 0;
    uint32_t resultCode = 1;
    uint64_t fromScreenId = 2;
    uint64_t toScreenId = 3;

    slController->NotifySessionTransferToTargetScreenEvent(info, resultCode, fromScreenId, toScreenId);
    usleep(WAIT_SYNC_IN_NS);
    EXPECT_EQ(myListener->event_, ISessionLifecycleListener::SessionLifecycleEvent::TRANSFER_TO_TARGET_SCREEN);
    EXPECT_EQ(myListener->payload_.persistentId_, info.persistentId_);
    EXPECT_EQ(myListener->payload_.fromScreenId_, fromScreenId);
    EXPECT_EQ(myListener->payload_.toScreenId_, toScreenId);
    EXPECT_EQ(myListener->payload_.resultCode_, resultCode);
}
} // namespace
} // namespace Rosen
} // namespace OHOS
