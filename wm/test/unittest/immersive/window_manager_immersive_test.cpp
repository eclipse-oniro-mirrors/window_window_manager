/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "iremote_object_mocker.h"
#include "scene_board_judgement.h"
#include "singleton_mocker.h"
#include "window_manager.cpp"
#include "window_manager.h"
#include "window_manager_hilog.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {

// Mock the functions in WindowAdapter.
class MockWindowAdapter : public WindowAdapter {
public:
    explicit MockWindowAdapter(const int32_t userId) : WindowAdapter(userId) {}
    ~MockWindowAdapter() = default;

    WMError RegisterWindowManagerAgent(WindowManagerAgentType type,
                                       const sptr<IWindowManagerAgent>& windowManagerAgent) override
    {
        return WMError::WM_OK;
    }

    WMError RegisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
                                              uint32_t interestInfo,
                                              const sptr<IWindowManagerAgent>& windowManagerAgent) override
    {
        return WMError::WM_OK;
    }

    WMError UnregisterWindowManagerAgent(WindowManagerAgentType type,
                                         const sptr<IWindowManagerAgent>& windowManagerAgent) override
    {
        return WMError::WM_OK;
    }

    WMError UnregisterWindowPropertyChangeAgent(WindowInfoKey windowInfoKey,
                                                uint32_t interestInfo,
                                                const sptr<IWindowManagerAgent>& windowManagerAgent)
    {
        return WMError::WM_OK;
    }

    WMError SetSpecificWindowZIndex(WindowType windowType, int32_t zIndex, bool updateMap)
    {
        return WMError::WM_OK;
    }
};

namespace {
class TestWindowSystemBarPropertyChangedListener : public IWindowSystemBarPropertyChangedListener {
public:
    int32_t count_ = 0;
    void OnWindowSystemBarPropertyChanged(WindowType type, const SystemBarProperty& systemBarProperty) override
    {
        count_ = 1;
        TLOGI(WmsLogTag::WMS_IMMS, "TestSystemBarChangedListener");
    }
};

class WindowManagerImmersiveTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

private:
    int32_t userId_ = 100;
    sptr<WindowManager> instance_ = nullptr;
    sptr<WindowAdapter> windowAdapter = nullptr;

    // use mock
    int32_t mockUserId_ = 200;
    sptr<WindowManager> mockInstance_ = nullptr;
    sptr<MockWindowAdapter> mockAdapter_ = nullptr;
};

void WindowManagerImmersiveTest::SetUpTestCase() {}

void WindowManagerImmersiveTest::TearDownTestCase() {}

void WindowManagerImmersiveTest::SetUp()
{
    instance_ = &WindowManager::GetInstance(userId_);
    windowAdapter = &WindowAdapter::GetInstance(userId_);

    // Note: To mock WindowAdapter, we can mock WindowAdapter by create a mocked adapter instance and
    // insert it to windowAdapterMap_. Then, WindowAdapter::GetInstance(mockUserId_) will get it.
    mockAdapter_ = sptr<MockWindowAdapter>::MakeSptr(mockUserId_);
    WindowAdapter::windowAdapterMap_[mockUserId_] = mockAdapter_;

    mockInstance_ = &WindowManager::GetInstance(mockUserId_);
}

void WindowManagerImmersiveTest::TearDown()
{
    WindowManager::RemoveInstanceByUserId(userId_);
    WindowManager::RemoveInstanceByUserId(mockUserId_);

    WindowAdapter::windowAdapterMap_.clear();
}

namespace {

/**
 * @tc.name: RegisterWindowSystemBarPropertyChangedListener
 * @tc.desc: check RegisterWindowSystemBarPropertyChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerImmersiveTest, RegisterWindowSystemBarPropertyChangedListener, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    auto oldWindowManagerAgent = instance_->pImpl_->windowSystemBarPropertyChangeAgent_;
    auto oldListeners = instance_->pImpl_->windowSystemBarPropertyChangedListeners_;
    instance_->pImpl_->windowSystemBarPropertyChangedListeners_.clear();
    instance_->pImpl_->windowSystemBarPropertyChangeAgent_ = nullptr;
    instance_->pImpl_->windowSystemBarPropertyChangedListeners_.clear();
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, instance_->RegisterWindowSystemBarPropertyChangedListener(nullptr));

    auto listener = sptr<TestWindowSystemBarPropertyChangedListener>::MakeSptr();
    EXPECT_EQ(WMError::WM_ERROR_SAMGR, instance_->RegisterWindowSystemBarPropertyChangedListener(listener));

    auto listener2 = sptr<TestWindowSystemBarPropertyChangedListener>::MakeSptr();
    instance_->pImpl_->windowSystemBarPropertyChangeAgent_ = new WindowManagerAgent();
    EXPECT_EQ(WMError::WM_ERROR_SAMGR, instance_->RegisterWindowSystemBarPropertyChangedListener(listener2));

    auto listener3 = sptr<TestWindowSystemBarPropertyChangedListener>::MakeSptr();
    instance_->RegisterWindowSystemBarPropertyChangedListener(listener3);

    instance_->pImpl_->windowSystemBarPropertyChangeAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->windowSystemBarPropertyChangedListeners_ = oldListeners;
}

/**
 * @tc.name: UnregisterWindowSystemBarPropertyChangedListener
 * @tc.desc: check UnregisterWindowSystemBarPropertyChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerImmersiveTest, UnregisterWindowSystemBarPropertyChangedListener, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->windowSystemBarPropertyChangeAgent_;
    auto oldListeners = instance_->pImpl_->windowSystemBarPropertyChangedListeners_;
    instance_->pImpl_->windowSystemBarPropertyChangedListeners_.clear();
    instance_->pImpl_->windowSystemBarPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    instance_->pImpl_->windowSystemBarPropertyChangedListeners_.clear();
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterWindowSystemBarPropertyChangedListener(nullptr));

    sptr<TestWindowSystemBarPropertyChangedListener> listener =
        sptr<TestWindowSystemBarPropertyChangedListener>::MakeSptr();
    instance_->pImpl_->windowSystemBarPropertyChangedListeners_.clear();
    EXPECT_EQ(WMError::WM_DO_NOTHING, instance_->UnregisterWindowSystemBarPropertyChangedListener(listener));

    instance_->pImpl_->windowSystemBarPropertyChangedListeners_.emplace_back(listener);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    EXPECT_EQ(WMError::WM_ERROR_SAMGR, instance_->UnregisterWindowSystemBarPropertyChangedListener(listener));
    instance_->pImpl_->windowSystemBarPropertyChangeAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->windowSystemBarPropertyChangedListeners_ = oldListeners;
}

/**
 * @tc.name: NotifyWindowSystemBarPropertyChange
 * @tc.desc: check NotifyWindowSystemBarPropertyChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerImmersiveTest, NotifyWindowSystemBarPropertyChange, TestSize.Level1)
{
    auto& windowManager = WindowManager::GetInstance();
    auto oldListeners = windowManager.pImpl_->windowSystemBarPropertyChangedListeners_;
    SystemBarProperty systemBarProperty;
    WindowManager::GetInstance().pImpl_->NotifyWindowSystemBarPropertyChange(
        WindowType::WINDOW_TYPE_STATUS_BAR, systemBarProperty);
    sptr<TestWindowSystemBarPropertyChangedListener> listener =
        sptr<TestWindowSystemBarPropertyChangedListener>::MakeSptr();
    windowManager.pImpl_->windowSystemBarPropertyChangedListeners_.emplace_back(listener);
    EXPECT_EQ(1, windowManager.pImpl_->windowSystemBarPropertyChangedListeners_.size());
    WindowManager::GetInstance().pImpl_->NotifyWindowSystemBarPropertyChange(
        WindowType::WINDOW_TYPE_STATUS_BAR, systemBarProperty);
    EXPECT_EQ(1, listener->count_);
    windowManager.pImpl_->windowSystemBarPropertyChangedListeners_ = oldListeners;
}

/**
 * @tc.name: RegisterSystemBarChangedListener01
 * @tc.desc: check RegisterSystemBarChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerImmersiveTest, RegisterSystemBarChangedListener01, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->systemBarChangedListenerAgent_;
    auto oldListeners = instance_->pImpl_->systemBarChangedListeners_;
    instance_->pImpl_->systemBarChangedListenerAgent_ = nullptr;
    instance_->pImpl_->systemBarChangedListeners_.clear();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->RegisterSystemBarChangedListener(nullptr));

    auto oldAgent = instance_->pImpl_->systemBarChangedListenerAgent_;

    instance_->pImpl_->systemBarChangedListenerAgent_ = nullptr;
    instance_->pImpl_->systemBarChangedListeners_.clear();
    WMError ret = instance_->RegisterSystemBarChangedListener(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    sptr<ISystemBarChangedListener> listener = sptr<TestSystemBarChangedListener>::MakeSptr();
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;

    ASSERT_EQ(WMError::WM_ERROR_SAMGR, instance_->RegisterSystemBarChangedListener(listener));
    ASSERT_EQ(0, instance_->pImpl_->systemBarChangedListeners_.size());

    // to check that the same listner can not be registered twice

    instance_->pImpl_->systemBarChangedListenerAgent_ = oldAgent;
    instance_->pImpl_->systemBarChangedListeners_ = oldListeners;
}

/**
 * @tc.name: UnregisterSystemBarChangedListener01
 * @tc.desc: check UnregisterSystemBarChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerImmersiveTest, UnregisterSystemBarChangedListener01, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    auto oldWindowManagerAgent = instance_->pImpl_->systemBarChangedListenerAgent_;
    auto oldListeners = instance_->pImpl_->systemBarChangedListeners_;
    instance_->pImpl_->systemBarChangedListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    instance_->pImpl_->systemBarChangedListeners_.clear();
    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterSystemBarChangedListener(nullptr));

    sptr<TestSystemBarChangedListener> listener1 = sptr<TestSystemBarChangedListener>::MakeSptr();
    sptr<TestSystemBarChangedListener> listener2 = sptr<TestSystemBarChangedListener>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterSystemBarChangedListener(listener1));
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    instance_->RegisterSystemBarChangedListener(listener1);
    instance_->RegisterSystemBarChangedListener(listener2);
    ASSERT_EQ(0, instance_->pImpl_->systemBarChangedListeners_.size());

    instance_->UnregisterSystemBarChangedListener(listener1);
    instance_->UnregisterSystemBarChangedListener(listener2);
    ASSERT_EQ(0, instance_->pImpl_->systemBarChangedListeners_.size());
    ASSERT_EQ(nullptr, instance_->pImpl_->systemBarChangedListenerAgent_);

    instance_->pImpl_->systemBarChangedListeners_.push_back(listener1);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterSystemBarChangedListener(listener1));
    ASSERT_EQ(0, instance_->pImpl_->systemBarChangedListeners_.size());

    instance_->pImpl_->systemBarChangedListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->systemBarChangedListeners_ = oldListeners;
}
}
} // namespace
} // namespace Rosen
} // namespace OHOS
