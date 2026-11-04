/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "parameters.h"

#include "iremote_object_mocker.h"
// the .cpp include gives the full Impl definition and the full adapter header;
// the lite adapter header must not be included in the same translation unit
#include "window_manager.cpp"
#include "window_manager_agent.h"
#include "window_manager_agent_proxy.h"
#include "window_manager_agent_stub.h"
#include "wm_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr int32_t TEST_WINDOW_ID = 11;
constexpr int32_t TEST_WINDOW_ID_2 = 22;

class GameAdaptiveListenerTester : public IGameAdaptiveResolutionListener {
public:
    GameAdaptiveListenerTester(int32_t& cancelledId, int32_t& availableId)
        : cancelledId_(cancelledId), availableId_(availableId) {}
    ~GameAdaptiveListenerTester() = default;
    void OnGameAdaptiveResolutionCancelled(int32_t windowId) override { cancelledId_ = windowId; }
    void OnGameAdaptiveResolutionAvailable(int32_t windowId) override { availableId_ = windowId; }
private:
    int32_t& cancelledId_;
    int32_t& availableId_;
};

class MockGameWindowAdapter : public WindowAdapter {
public:
    explicit MockGameWindowAdapter(const int32_t userId) : WindowAdapter(userId) {}
    ~MockGameWindowAdapter() = default;

    WMError RegisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override
    {
        registerCount_++;
        return registerResult_;
    }

    WMError UnregisterWindowManagerAgent(WindowManagerAgentType type,
        const sptr<IWindowManagerAgent>& windowManagerAgent) override
    {
        unregisterCount_++;
        return unregisterResult_;
    }

    WMError SetGameAdaptiveResolutionForWindow(const GameAdaptiveResolutionConfig& config) override
    {
        lastConfig_ = config;
        return setConfigResult_;
    }

    int registerCount_ = 0;
    int unregisterCount_ = 0;
    WMError registerResult_ = WMError::WM_OK;
    WMError unregisterResult_ = WMError::WM_OK;
    WMError setConfigResult_ = WMError::WM_OK;
    GameAdaptiveResolutionConfig lastConfig_;
};
}

class GameAdaptiveAgentTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    // full mock side: adapter overrides return ok (the default instance plays the fail side)
    int32_t fullMockUserId_ = 201;
    sptr<MockGameWindowAdapter> fullMockAdapter_;
    std::string isConcurrentuser_;
};

void GameAdaptiveAgentTest::SetUpTestCase() {}

void GameAdaptiveAgentTest::TearDownTestCase() {}

void GameAdaptiveAgentTest::SetUp()
{
    isConcurrentuser_ = OHOS::system::GetParameter("persist.dms.concurrentuser", "");
    OHOS::system::SetParameter("persist.dms.concurrentuser", "true");
    fullMockAdapter_ = sptr<MockGameWindowAdapter>::MakeSptr(fullMockUserId_);
    WindowAdapter::windowAdapterMap_[fullMockUserId_] = fullMockAdapter_;
}

void GameAdaptiveAgentTest::TearDown()
{
    WindowManager::RemoveInstanceByUserId(fullMockUserId_);
    WindowAdapter::windowAdapterMap_.erase(fullMockUserId_);
    OHOS::system::SetParameter("persist.dms.concurrentuser", isConcurrentuser_);
}

namespace {
/**
 * @tc.name: FullRegisterListenerAgentMode
 * @tc.desc: register branches: null, adapter failure, success, duplicate
 * @tc.type: FUNC
 */
HWTEST_F(GameAdaptiveAgentTest, FullRegisterListenerAgentMode, TestSize.Level1)
{
    auto& windowManager = WindowManager::GetInstance();
    auto& mockManager = WindowManager::GetInstance(fullMockUserId_);
    int32_t cancelledId = -1;
    int32_t availableId = -1;
    auto listener = sptr<GameAdaptiveListenerTester>::MakeSptr(cancelledId, availableId);

    // null listener
    EXPECT_EQ(windowManager.RegisterGameAdaptiveResolutionListener(nullptr), WMError::WM_ERROR_NULLPTR);
    EXPECT_EQ(mockManager.RegisterGameAdaptiveResolutionListener(nullptr), WMError::WM_ERROR_NULLPTR);

    // adapter failure clears the agent and keeps the local table empty
    EXPECT_NE(windowManager.RegisterGameAdaptiveResolutionListener(listener), WMError::WM_OK);
    EXPECT_EQ(windowManager.pImpl_->gameAdaptiveListenerAgent_, nullptr);
    EXPECT_TRUE(windowManager.pImpl_->gameAdaptiveListeners_.empty());

    // mocked adapter: success
    EXPECT_EQ(mockManager.RegisterGameAdaptiveResolutionListener(listener), WMError::WM_OK);
    EXPECT_NE(mockManager.pImpl_->gameAdaptiveListenerAgent_, nullptr);
    EXPECT_EQ(mockManager.pImpl_->gameAdaptiveListeners_.size(), 1u);
    EXPECT_EQ(fullMockAdapter_->registerCount_, 1);

    // duplicate register keeps a single entry; the agent ipc still fires
    EXPECT_EQ(mockManager.RegisterGameAdaptiveResolutionListener(listener), WMError::WM_OK);
    EXPECT_EQ(mockManager.pImpl_->gameAdaptiveListeners_.size(), 1u);
    EXPECT_EQ(fullMockAdapter_->registerCount_, 2);

    mockManager.pImpl_->gameAdaptiveListeners_.erase(listener);
    mockManager.pImpl_->gameAdaptiveListenerAgent_ = nullptr;
}

/**
 * @tc.name: FullUnregisterListenerAgentMode
 * @tc.desc: unregister branches: null, unknown listener, last listener, adapter failure
 * @tc.type: FUNC
 */
HWTEST_F(GameAdaptiveAgentTest, FullUnregisterListenerAgentMode, TestSize.Level1)
{
    auto& mockManager = WindowManager::GetInstance(fullMockUserId_);
    int32_t cancelledIdA = -1;
    int32_t availableIdA = -1;
    int32_t cancelledIdB = -1;
    int32_t availableIdB = -1;
    auto listenerA = sptr<GameAdaptiveListenerTester>::MakeSptr(cancelledIdA, availableIdA);
    auto listenerB = sptr<GameAdaptiveListenerTester>::MakeSptr(cancelledIdB, availableIdB);

    EXPECT_EQ(mockManager.UnregisterGameAdaptiveResolutionListener(nullptr), WMError::WM_ERROR_NULLPTR);

    ASSERT_EQ(mockManager.RegisterGameAdaptiveResolutionListener(listenerA), WMError::WM_OK);
    ASSERT_EQ(mockManager.RegisterGameAdaptiveResolutionListener(listenerB), WMError::WM_OK);

    // unknown listener returns ok without touching the agent
    int32_t cancelledIdC = -1;
    int32_t availableIdC = -1;
    auto listenerC = sptr<GameAdaptiveListenerTester>::MakeSptr(cancelledIdC, availableIdC);
    EXPECT_EQ(mockManager.UnregisterGameAdaptiveResolutionListener(listenerC), WMError::WM_OK);
    EXPECT_NE(mockManager.pImpl_->gameAdaptiveListenerAgent_, nullptr);
    EXPECT_EQ(fullMockAdapter_->unregisterCount_, 0);

    // sibling keeps the agent registered
    EXPECT_EQ(mockManager.UnregisterGameAdaptiveResolutionListener(listenerA), WMError::WM_OK);
    EXPECT_NE(mockManager.pImpl_->gameAdaptiveListenerAgent_, nullptr);
    EXPECT_EQ(fullMockAdapter_->unregisterCount_, 0);

    // last listener unregisters the agent
    EXPECT_EQ(mockManager.UnregisterGameAdaptiveResolutionListener(listenerB), WMError::WM_OK);
    EXPECT_EQ(mockManager.pImpl_->gameAdaptiveListenerAgent_, nullptr);
    EXPECT_EQ(fullMockAdapter_->unregisterCount_, 1);

    // adapter failure on the last unregister keeps the agent for retry
    ASSERT_EQ(mockManager.RegisterGameAdaptiveResolutionListener(listenerA), WMError::WM_OK);
    fullMockAdapter_->unregisterResult_ = WMError::WM_ERROR_SAMGR;
    EXPECT_EQ(mockManager.UnregisterGameAdaptiveResolutionListener(listenerA), WMError::WM_ERROR_SAMGR);
    EXPECT_NE(mockManager.pImpl_->gameAdaptiveListenerAgent_, nullptr);
    EXPECT_TRUE(mockManager.pImpl_->gameAdaptiveListeners_.empty());

    // retry round trip clears the agent
    ASSERT_EQ(mockManager.RegisterGameAdaptiveResolutionListener(listenerA), WMError::WM_OK);
    fullMockAdapter_->unregisterResult_ = WMError::WM_OK;
    EXPECT_EQ(mockManager.UnregisterGameAdaptiveResolutionListener(listenerA), WMError::WM_OK);
    EXPECT_EQ(mockManager.pImpl_->gameAdaptiveListenerAgent_, nullptr);
}

/**
 * @tc.name: FullNotifyFanOut
 * @tc.desc: the impl notify fans out to all listeners and skips null entries
 * @tc.type: FUNC
 */
HWTEST_F(GameAdaptiveAgentTest, FullNotifyFanOut, TestSize.Level1)
{
    auto& mockManager = WindowManager::GetInstance(fullMockUserId_);
    int32_t cancelledIdA = -1;
    int32_t availableIdA = -1;
    int32_t cancelledIdB = -1;
    int32_t availableIdB = -1;
    auto listenerA = sptr<GameAdaptiveListenerTester>::MakeSptr(cancelledIdA, availableIdA);
    auto listenerB = sptr<GameAdaptiveListenerTester>::MakeSptr(cancelledIdB, availableIdB);
    mockManager.pImpl_->gameAdaptiveListeners_.insert(listenerA);
    mockManager.pImpl_->gameAdaptiveListeners_.insert(listenerB);
    mockManager.pImpl_->gameAdaptiveListeners_.insert(nullptr);

    mockManager.NotifyGameAdaptiveResolutionCancelled(TEST_WINDOW_ID);
    EXPECT_EQ(cancelledIdA, TEST_WINDOW_ID);
    EXPECT_EQ(cancelledIdB, TEST_WINDOW_ID);
    mockManager.NotifyGameAdaptiveResolutionAvailable(TEST_WINDOW_ID_2);
    EXPECT_EQ(availableIdA, TEST_WINDOW_ID_2);
    EXPECT_EQ(availableIdB, TEST_WINDOW_ID_2);

    mockManager.pImpl_->gameAdaptiveListeners_.clear();
}

/**
 * @tc.name: FullAgentForwarding
 * @tc.desc: WindowManagerAgent forwards the callbacks to the user-bound instance
 * @tc.type: FUNC
 */
HWTEST_F(GameAdaptiveAgentTest, FullAgentForwarding, TestSize.Level1)
{
    auto& mockManager = WindowManager::GetInstance(fullMockUserId_);
    int32_t cancelledId = -1;
    int32_t availableId = -1;
    auto listener = sptr<GameAdaptiveListenerTester>::MakeSptr(cancelledId, availableId);
    mockManager.pImpl_->gameAdaptiveListeners_.insert(listener);
    auto agent = sptr<WindowManagerAgent>::MakeSptr(fullMockUserId_);

    agent->OnGameAdaptiveResolutionCancelled(TEST_WINDOW_ID);
    EXPECT_EQ(cancelledId, TEST_WINDOW_ID);
    agent->OnGameAdaptiveResolutionAvailable(TEST_WINDOW_ID_2);
    EXPECT_EQ(availableId, TEST_WINDOW_ID_2);

    mockManager.pImpl_->gameAdaptiveListeners_.erase(listener);
}

/**
 * @tc.name: AgentStubGameAdaptiveMessages
 * @tc.desc: stub decodes the two new messages and forwards to the agent callbacks
 * @tc.type: FUNC
 */
HWTEST_F(GameAdaptiveAgentTest, AgentStubGameAdaptiveMessages, TestSize.Level1)
{
    auto& mockManager = WindowManager::GetInstance(fullMockUserId_);
    int32_t cancelledId = -1;
    int32_t availableId = -1;
    auto listener = sptr<GameAdaptiveListenerTester>::MakeSptr(cancelledId, availableId);
    mockManager.pImpl_->gameAdaptiveListeners_.insert(listener);
    sptr<WindowManagerAgent> stub = sptr<WindowManagerAgent>::MakeSptr(fullMockUserId_);

    auto cancelledCode = static_cast<uint32_t>(
        IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_ON_GAME_ADAPTIVE_RESOLUTION_CANCELLED);
    auto availableCode = static_cast<uint32_t>(
        IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_ON_GAME_ADAPTIVE_RESOLUTION_AVAILABLE);

    // bad interface token
    MessageParcel reply;
    MessageOption option;
    MessageParcel badToken;
    badToken.WriteInterfaceToken(u"error.GetDescriptor");
    EXPECT_EQ(stub->OnRemoteRequest(cancelledCode, badToken, reply, option),
        static_cast<int>(ERR_TRANSACTION_FAILED));

    // missing windowId
    MessageParcel noId;
    noId.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());
    EXPECT_EQ(stub->OnRemoteRequest(cancelledCode, noId, reply, option),
        static_cast<int>(ERR_INVALID_DATA));

    // cancelled: valid parcel reaches the listener
    MessageParcel cancelledData;
    cancelledData.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());
    cancelledData.WriteInt32(TEST_WINDOW_ID);
    EXPECT_EQ(stub->OnRemoteRequest(cancelledCode, cancelledData, reply, option), 0);
    EXPECT_EQ(cancelledId, TEST_WINDOW_ID);

    // available: valid parcel reaches the listener
    MessageParcel availableData;
    availableData.WriteInterfaceToken(WindowManagerAgentStub::GetDescriptor());
    availableData.WriteInt32(TEST_WINDOW_ID_2);
    EXPECT_EQ(stub->OnRemoteRequest(availableCode, availableData, reply, option), 0);
    EXPECT_EQ(availableId, TEST_WINDOW_ID_2);

    mockManager.pImpl_->gameAdaptiveListeners_.erase(listener);
}

/**
 * @tc.name: AgentProxyGameAdaptiveMessages
 * @tc.desc: proxy sends the two new messages with the right transaction codes
 * @tc.type: FUNC
 */
HWTEST_F(GameAdaptiveAgentTest, AgentProxyGameAdaptiveMessages, TestSize.Level1)
{
    auto cancelledCode = static_cast<uint32_t>(
        IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_ON_GAME_ADAPTIVE_RESOLUTION_CANCELLED);
    auto availableCode = static_cast<uint32_t>(
        IWindowManagerAgent::WindowManagerAgentMsg::TRANS_ID_ON_GAME_ADAPTIVE_RESOLUTION_AVAILABLE);

    sptr<RemoteObjectMocker> remote = sptr<RemoteObjectMocker>::MakeSptr();
    ASSERT_NE(remote, nullptr);
    auto proxy = sptr<WindowManagerAgentProxy>::MakeSptr(remote);
    ASSERT_NE(proxy, nullptr);

    EXPECT_CALL(*remote, SendRequest(cancelledCode, _, _, _)).Times(1);
    proxy->OnGameAdaptiveResolutionCancelled(TEST_WINDOW_ID);

    EXPECT_CALL(*remote, SendRequest(availableCode, _, _, _)).Times(1);
    proxy->OnGameAdaptiveResolutionAvailable(TEST_WINDOW_ID_2);

    // null remote: both callbacks return before any send
    sptr<WindowManagerAgentProxy> nullProxy = sptr<WindowManagerAgentProxy>::MakeSptr(nullptr);
    ASSERT_NE(nullProxy, nullptr);
    nullProxy->OnGameAdaptiveResolutionCancelled(TEST_WINDOW_ID);
    nullProxy->OnGameAdaptiveResolutionAvailable(TEST_WINDOW_ID_2);

    // send failure only logs (both callbacks)
    EXPECT_CALL(*remote, SendRequest(cancelledCode, _, _, _)).WillOnce(Return(1));
    proxy->OnGameAdaptiveResolutionCancelled(TEST_WINDOW_ID);
    EXPECT_CALL(*remote, SendRequest(availableCode, _, _, _)).WillOnce(Return(1));
    proxy->OnGameAdaptiveResolutionAvailable(TEST_WINDOW_ID_2);
}
}
}
}
