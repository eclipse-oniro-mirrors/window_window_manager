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

#include "pixel_map.h"
#include "session_listener_controller.h"

#include <gtest/gtest.h>

#include "display_manager_adapter.h"
#include "mission_listener_stub.h"
#include "singleton_container.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Media;
namespace OHOS {
namespace Rosen {
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
};

class SessionListenerControllerTest : public testing::Test {
  public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<SessionListenerController> slController;
private:
    static constexpr uint32_t WAIT_SYNC_IN_NS = 200000;
};


void SessionListenerControllerTest::SetUpTestCase()
{
}

void SessionListenerControllerTest::TearDownTestCase()
{
}

void SessionListenerControllerTest::SetUp()
{
    slController = std::make_shared<SessionListenerController>();
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
HWTEST_F(SessionListenerControllerTest, AddSessionListener, Function | SmallTest | Level2)
{
    sptr<ISessionListener> listener;
    ASSERT_EQ(listener, nullptr);
    WSError res = slController->AddSessionListener(listener);
    ASSERT_EQ(res, WSError::WS_ERROR_INVALID_PARAM);

    slController->Init();
    listener = new MyMissionListener();
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
HWTEST_F(SessionListenerControllerTest, DelSessionListener, Function | SmallTest | Level2)
{
    sptr<ISessionListener> listener;
    ASSERT_EQ(listener, nullptr);
    slController->DelSessionListener(listener);
    int32_t persistentId = 1;
    slController->NotifySessionLabelUpdated(persistentId);
    ASSERT_EQ(persistentId, 1);

    slController->Init();
    listener = new MyMissionListener();
    slController->DelSessionListener(listener);
    EXPECT_NE(nullptr, listener);
}

/**
 * @tc.name: NotifySessionCreated
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionCreated, Function | SmallTest | Level2)
{
    int32_t persistentId = -1;
    ASSERT_NE(slController, nullptr);
    slController->NotifySessionCreated(persistentId);

    persistentId = 1;
    slController->NotifySessionCreated(persistentId);

    slController->Init();
    slController->NotifySessionCreated(persistentId);
}

/**
 * @tc.name: NotifySessionDestroyed
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionDestroyed, Function | SmallTest | Level2)
{
    int32_t persistentId = -1;
    ASSERT_NE(slController, nullptr);
    slController->NotifySessionDestroyed(persistentId);

    persistentId = 1;
    slController->NotifySessionDestroyed(persistentId);

    slController->Init();
    slController->NotifySessionDestroyed(persistentId);
}

/**
 * @tc.name: HandleUnInstallApp1
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, HandleUnInstallApp1, Function | SmallTest | Level2)
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
HWTEST_F(SessionListenerControllerTest, HandleUnInstallApp2, Function | SmallTest | Level2)
{
    std::list<int32_t> sessions;
    sessions.push_front(1);
    slController->HandleUnInstallApp(sessions);
    slController->Init();
    slController->HandleUnInstallApp(sessions);
    EXPECT_NE(0, sessions.size());
    int32_t persistentId = 1;
    slController->NotifySessionLabelUpdated(persistentId);
    ASSERT_EQ(persistentId, 1);
}

/**
 * @tc.name: NotifySessionSnapshotChanged
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionSnapshotChanged, Function | SmallTest | Level2)
{
    int32_t persistentId = -1;
    ASSERT_NE(slController, nullptr);
    slController->NotifySessionSnapshotChanged(persistentId);

    persistentId = 1;
    slController->NotifySessionSnapshotChanged(persistentId);

    slController->Init();
    slController->NotifySessionSnapshotChanged(persistentId);
}

/**
 * @tc.name: NotifySessionMovedToFront
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionMovedToFront, Function | SmallTest | Level2)
{
    int32_t persistentId = -1;
    ASSERT_NE(slController, nullptr);
    slController->NotifySessionMovedToFront(persistentId);

    persistentId = 1;
    slController->NotifySessionMovedToFront(persistentId);

    slController->Init();
    slController->NotifySessionSnapshotChanged(persistentId);
}

/**
 * @tc.name: NotifySessionFocused
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionFocused, Function | SmallTest | Level2)
{
    int32_t persistentId = -1;
    ASSERT_NE(slController, nullptr);
    slController->NotifySessionFocused(persistentId);

    persistentId = 1;
    slController->NotifySessionFocused(persistentId);

    slController->Init();
    slController->NotifySessionSnapshotChanged(persistentId);
}

/**
 * @tc.name: NotifySessionUnfocused
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionUnfocused, Function | SmallTest | Level2)
{
    int32_t persistentId = -1;
    ASSERT_NE(slController, nullptr);
    slController->NotifySessionUnfocused(persistentId);

    persistentId = 1;
    slController->NotifySessionUnfocused(persistentId);

    slController->Init();
    slController->NotifySessionSnapshotChanged(persistentId);
}

/**
 * @tc.name: NotifySessionClosed
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionClosed, Function | SmallTest | Level2)
{
    int32_t persistentId = -1;
    ASSERT_NE(slController, nullptr);
    slController->NotifySessionClosed(persistentId);

    persistentId = 1;
    slController->NotifySessionClosed(persistentId);

    slController->Init();
    slController->NotifySessionSnapshotChanged(persistentId);
}

/**
 * @tc.name: NotifySessionLabelUpdated
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionLabelUpdated, Function | SmallTest | Level2)
{
    int32_t persistentId = -1;
    ASSERT_NE(slController, nullptr);
    slController->NotifySessionLabelUpdated(persistentId);

    persistentId = 1;
    slController->NotifySessionLabelUpdated(persistentId);

    slController->Init();
    slController->NotifySessionSnapshotChanged(persistentId);
}

/**
 * @tc.name: OnListenerDied
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, OnListenerDied, Function | SmallTest | Level2)
{
    sptr<IRemoteObject> remote;
    if (slController == nullptr) {
        return;
    }
    slController->OnListenerDied(remote);
    EXPECT_EQ(nullptr, remote);

    if (SingletonContainer::Get<ScreenManagerAdapter>().InitDMSProxy()) {
        remote = SingletonContainer::Get<ScreenManagerAdapter>().displayManagerServiceProxy_->AsObject();
        slController->OnListenerDied(remote);
        EXPECT_NE(nullptr, remote);
    }
}

/**
 * @tc.name: NotifySessionIconChanged
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(SessionListenerControllerTest, NotifySessionIconChanged, Function | SmallTest | Level2)
{
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

    persistentId = 1;
    slController->NotifySessionIconChanged(persistentId, icon);

    slController->Init();
    slController->NotifySessionIconChanged(persistentId, icon);
    EXPECT_NE(nullptr, slController->taskScheduler_);
}

/**
 * @tc.name: ListenerDeathRecipient
 * @tc.desc: ListenerDeathRecipient class test
 * @tc.type: CLASS
 */
HWTEST_F(SessionListenerControllerTest, ListenerDeathRecipient, Function | SmallTest | Level2)
{
    GTEST_LOG_(INFO) << "TaskSchedulerText: task_scheduler_test001 start";
    EXPECT_EQ(nullptr,  slController->listenerDeathRecipient_);
    slController->Init();
    sptr<ISessionListener> listener = new MyMissionListener();
    slController->AddSessionListener(listener);
    EXPECT_NE(nullptr,  slController->listenerDeathRecipient_);

    if (SingletonContainer::Get<ScreenManagerAdapter>().InitDMSProxy()) {
        sptr<IRemoteObject> remote;
        remote = SingletonContainer::Get<ScreenManagerAdapter>().displayManagerServiceProxy_->AsObject();
        slController->listenerDeathRecipient_->OnRemoteDied(remote);
        EXPECT_NE(nullptr, remote);
    }
    GTEST_LOG_(INFO) << "TaskSchedulerText: task_scheduler_test001 end";
}
} // namespace
} // namespace Rosen
} // namespace OHOS