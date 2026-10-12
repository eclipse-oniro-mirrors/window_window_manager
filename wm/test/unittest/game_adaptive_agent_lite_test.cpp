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

// the .cpp include gives the lite Impl definition and the lite adapter header;
// the full adapter header must not be included in the same translation unit
#include "window_manager_lite.cpp"
#include "window_manager_agent_lite.h"
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

class MockGameWindowAdapterLite : public WindowAdapterLite {
public:
    explicit MockGameWindowAdapterLite(const int32_t userId) : WindowAdapterLite(userId) {}
    ~MockGameWindowAdapterLite() = default;

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

class GameAdaptiveAgentLiteTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    // lite fail side: proxy init fails, register/unregister/set report errors
    int32_t liteUserId_ = 102;
    // lite mock side: adapter overrides return ok
    int32_t liteMockUserId_ = 202;
    sptr<MockGameWindowAdapterLite> liteMockAdapter_;
    sptr<WindowAdapterLite> liteFailAdapter_;
    std::string isConcurrentuser_;
};

void GameAdaptiveAgentLiteTest::SetUpTestCase() {}

void GameAdaptiveAgentLiteTest::TearDownTestCase() {}

void GameAdaptiveAgentLiteTest::SetUp()
{
    isConcurrentuser_ = OHOS::system::GetParameter("persist.dms.concurrentuser", "");
    OHOS::system::SetParameter("persist.dms.concurrentuser", "true");

    liteFailAdapter_ = sptr<WindowAdapterLite>::MakeSptr(liteUserId_);
    // make the non-mocked adapter fail on every ipc entry
    liteFailAdapter_->isProxyValid_ = true;
    liteFailAdapter_->windowManagerServiceProxy_ = nullptr;
    WindowAdapterLite::windowAdapterLiteMap_[liteUserId_] = liteFailAdapter_;

    liteMockAdapter_ = sptr<MockGameWindowAdapterLite>::MakeSptr(liteMockUserId_);
    WindowAdapterLite::windowAdapterLiteMap_[liteMockUserId_] = liteMockAdapter_;
}

void GameAdaptiveAgentLiteTest::TearDown()
{
    WindowManagerLite::RemoveInstanceByUserId(liteUserId_);
    WindowManagerLite::RemoveInstanceByUserId(liteMockUserId_);
    WindowAdapterLite::windowAdapterLiteMap_.erase(liteUserId_);
    WindowAdapterLite::windowAdapterLiteMap_.erase(liteMockUserId_);
    OHOS::system::SetParameter("persist.dms.concurrentuser", isConcurrentuser_);
}

namespace {
/**
 * @tc.name: LiteSetConfigFacade
 * @tc.desc: lite set forwards to the adapter and reports its result
 * @tc.type: FUNC
 */
HWTEST_F(GameAdaptiveAgentLiteTest, LiteSetConfigFacade, TestSize.Level1)
{
    GameAdaptiveResolutionConfig config;
    config.enable_ = true;
    config.scaleRatio_ = 0.5f;
    config.windowId_ = TEST_WINDOW_ID;

    // fail side: no proxy, samgr error
    auto& failManager = WindowManagerLite::GetInstance(liteUserId_);
    EXPECT_EQ(failManager.SetGameAdaptiveResolutionConfig(config), WMError::WM_ERROR_SAMGR);

    // mock side: adapter ok, config passed through
    auto& mockManager = WindowManagerLite::GetInstance(liteMockUserId_);
    EXPECT_EQ(mockManager.SetGameAdaptiveResolutionConfig(config), WMError::WM_OK);
    EXPECT_EQ(liteMockAdapter_->lastConfig_.windowId_, TEST_WINDOW_ID);
    EXPECT_EQ(liteMockAdapter_->lastConfig_.scaleRatio_, 0.5f);

    liteMockAdapter_->setConfigResult_ = WMError::WM_ERROR_INVALID_OPERATION;
    EXPECT_EQ(mockManager.SetGameAdaptiveResolutionConfig(config), WMError::WM_ERROR_INVALID_OPERATION);
    liteMockAdapter_->setConfigResult_ = WMError::WM_OK;
}

/**
 * @tc.name: LiteRegisterUnregisterAgentMode
 * @tc.desc: lite register/unregister mirror the full side branch by branch
 * @tc.type: FUNC
 */
HWTEST_F(GameAdaptiveAgentLiteTest, LiteRegisterUnregisterAgentMode, TestSize.Level1)
{
    auto& failManager = WindowManagerLite::GetInstance(liteUserId_);
    auto& mockManager = WindowManagerLite::GetInstance(liteMockUserId_);
    int32_t cancelledId = -1;
    int32_t availableId = -1;
    auto listener = sptr<GameAdaptiveListenerTester>::MakeSptr(cancelledId, availableId);

    // null listener
    EXPECT_EQ(mockManager.RegisterGameAdaptiveResolutionListener(nullptr), WMError::WM_ERROR_NULLPTR);
    EXPECT_EQ(mockManager.UnregisterGameAdaptiveResolutionListener(nullptr), WMError::WM_ERROR_NULLPTR);

    // adapter failure clears the agent and keeps the table empty
    EXPECT_NE(failManager.RegisterGameAdaptiveResolutionListener(listener), WMError::WM_OK);
    EXPECT_EQ(failManager.pImpl_->gameAdaptiveListenerAgent_, nullptr);
    EXPECT_TRUE(failManager.pImpl_->gameAdaptiveListeners_.empty());

    // mocked adapter: success and duplicate
    EXPECT_EQ(mockManager.RegisterGameAdaptiveResolutionListener(listener), WMError::WM_OK);
    EXPECT_NE(mockManager.pImpl_->gameAdaptiveListenerAgent_, nullptr);
    EXPECT_EQ(mockManager.pImpl_->gameAdaptiveListeners_.size(), 1u);
    EXPECT_EQ(liteMockAdapter_->registerCount_, 1);
    EXPECT_EQ(mockManager.RegisterGameAdaptiveResolutionListener(listener), WMError::WM_OK);
    EXPECT_EQ(mockManager.pImpl_->gameAdaptiveListeners_.size(), 1u);
    EXPECT_EQ(liteMockAdapter_->registerCount_, 2);

    // unknown listener returns ok without unregistering the agent
    int32_t cancelledIdB = -1;
    int32_t availableIdB = -1;
    auto listenerB = sptr<GameAdaptiveListenerTester>::MakeSptr(cancelledIdB, availableIdB);
    EXPECT_EQ(mockManager.UnregisterGameAdaptiveResolutionListener(listenerB), WMError::WM_OK);
    EXPECT_NE(mockManager.pImpl_->gameAdaptiveListenerAgent_, nullptr);

    // last listener unregisters the agent
    EXPECT_EQ(mockManager.UnregisterGameAdaptiveResolutionListener(listener), WMError::WM_OK);
    EXPECT_EQ(mockManager.pImpl_->gameAdaptiveListenerAgent_, nullptr);
    EXPECT_EQ(liteMockAdapter_->unregisterCount_, 1);

    // adapter failure on the last unregister keeps the agent for retry
    ASSERT_EQ(mockManager.RegisterGameAdaptiveResolutionListener(listener), WMError::WM_OK);
    liteMockAdapter_->unregisterResult_ = WMError::WM_ERROR_SAMGR;
    EXPECT_EQ(mockManager.UnregisterGameAdaptiveResolutionListener(listener), WMError::WM_ERROR_SAMGR);
    EXPECT_NE(mockManager.pImpl_->gameAdaptiveListenerAgent_, nullptr);
    EXPECT_TRUE(mockManager.pImpl_->gameAdaptiveListeners_.empty());

    // retry round trip clears the agent
    ASSERT_EQ(mockManager.RegisterGameAdaptiveResolutionListener(listener), WMError::WM_OK);
    liteMockAdapter_->unregisterResult_ = WMError::WM_OK;
    EXPECT_EQ(mockManager.UnregisterGameAdaptiveResolutionListener(listener), WMError::WM_OK);
    EXPECT_EQ(mockManager.pImpl_->gameAdaptiveListenerAgent_, nullptr);
}

/**
 * @tc.name: LiteNotifyFanOut
 * @tc.desc: lite impl notify fans out to all listeners and skips null entries
 * @tc.type: FUNC
 */
HWTEST_F(GameAdaptiveAgentLiteTest, LiteNotifyFanOut, TestSize.Level1)
{
    auto& mockManager = WindowManagerLite::GetInstance(liteMockUserId_);
    int32_t cancelledId = -1;
    int32_t availableId = -1;
    auto listener = sptr<GameAdaptiveListenerTester>::MakeSptr(cancelledId, availableId);
    mockManager.pImpl_->gameAdaptiveListeners_.insert(listener);
    mockManager.pImpl_->gameAdaptiveListeners_.insert(nullptr);

    mockManager.NotifyGameAdaptiveResolutionCancelled(TEST_WINDOW_ID);
    EXPECT_EQ(cancelledId, TEST_WINDOW_ID);
    mockManager.NotifyGameAdaptiveResolutionAvailable(TEST_WINDOW_ID_2);
    EXPECT_EQ(availableId, TEST_WINDOW_ID_2);

    mockManager.pImpl_->gameAdaptiveListeners_.clear();
}

/**
 * @tc.name: LiteAgentForwarding
 * @tc.desc: WindowManagerAgentLite forwards the callbacks to the user-bound lite instance
 * @tc.type: FUNC
 */
HWTEST_F(GameAdaptiveAgentLiteTest, LiteAgentForwarding, TestSize.Level1)
{
    auto& mockManager = WindowManagerLite::GetInstance(liteMockUserId_);
    int32_t cancelledId = -1;
    int32_t availableId = -1;
    auto listener = sptr<GameAdaptiveListenerTester>::MakeSptr(cancelledId, availableId);
    mockManager.pImpl_->gameAdaptiveListeners_.insert(listener);
    auto agent = sptr<WindowManagerAgentLite>::MakeSptr(liteMockUserId_);

    agent->OnGameAdaptiveResolutionCancelled(TEST_WINDOW_ID);
    EXPECT_EQ(cancelledId, TEST_WINDOW_ID);
    agent->OnGameAdaptiveResolutionAvailable(TEST_WINDOW_ID_2);
    EXPECT_EQ(availableId, TEST_WINDOW_ID_2);

    mockManager.pImpl_->gameAdaptiveListeners_.erase(listener);
}
}
}
}
