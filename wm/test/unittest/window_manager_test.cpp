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
#include "scene_session_manager.h"
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

    WMError SetSpecificWindowZIndex(WindowType windowType, int32_t zIndex)
    {
        return WMError::WM_OK;
    }
};

namespace {
    std::string g_errLog;
    void MyLogCallback(const LogType type, const LogLevel level, const unsigned int domain, const char *tag,
        const char *msg)
    {
        g_errLog = msg;
    }

class TestCameraFloatWindowChangedListener : public ICameraFloatWindowChangedListener {
public:
    void OnCameraFloatWindowChange(uint32_t accessTokenId, bool isShowing) override
    {
        WLOGI("TestCameraFloatWindowChangedListener [%{public}u, %{public}u]", accessTokenId, isShowing);
    };
};

class TestVisibilityChangedListener : public IVisibilityChangedListener {
public:
    void OnWindowVisibilityChanged(const std::vector<sptr<WindowVisibilityInfo>>& windowVisibilityInfo) override
    {
        WLOGI("TestVisibilityChangedListener");
    };
};

class TestSystemBarChangedListener : public ISystemBarChangedListener {
public:
    void OnSystemBarPropertyChange(DisplayId displayId, const SystemBarRegionTints& tints) override
    {
        WLOGI("TestSystemBarChangedListener");
    };
};

class TestWindowSystemBarPropertyChangedListener : public IWindowSystemBarPropertyChangedListener {
public:
    int32_t count_ = 0;
    void OnWindowSystemBarPropertyChanged(WindowType type, const SystemBarProperty& systemBarProperty) override
    {
        count_ = 1;
        TLOGI(WmsLogTag::WMS_IMMS, "TestSystemBarChangedListener");
    }
};

class TestWindowUpdateListener : public IWindowUpdateListener {
public:
    void OnWindowUpdate(const std::vector<sptr<AccessibilityWindowInfo>>& infos, WindowUpdateType type) override
    {
        WLOGI("TestWindowUpdateListener");
    };
};

class MockWindowInfoChangedListener : public IWindowInfoChangedListener {
public:
    int32_t count_ = 0;

    void OnWindowInfoChanged(const std::vector<std::unordered_map<WindowInfoKey,
        WindowChangeInfoType>>& windowInfoList) override
    {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "MockWindowInfoChangedListener");
        ++count_;
    };
};

class TestWindowDisplayIdChangeListener : public IWindowInfoChangedListener {
public:
    void OnWindowInfoChanged(
        const WindowInfoList& windowInfoList) override
    {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "TestWindowDisplayIdChangeListener");
    };
};

class TestWindowGlobalRectChangedListener : public IWindowInfoChangedListener {
public:
    void OnWindowInfoChanged(
        const WindowInfoList& windowInfoList) override
    {
        TLOGI(WmsLogTag::WMS_ATTRIBUTE, "TestWindowGlobalRectChangedListener");
    };
};

class TestWindowModeChangedListener : public IWindowModeChangedListener {
public:
    void OnWindowModeUpdate(WindowModeType mode) override
    {
        WLOGI("TestWindowModeChangedListener");
    };
};

class TestWaterMarkFlagChangeListener : public IWaterMarkFlagChangedListener {
public:
    void OnWaterMarkFlagUpdate(bool showWaterMark) override
    {
        WLOGI("TestWaterMarkFlagChangeListener");
    };
};

class TestGestureNavigationEnabledChangedListener : public IGestureNavigationEnabledChangedListener {
public:
    void OnGestureNavigationEnabledUpdate(bool enable) override
    {
        WLOGI("TestGestureNavigationEnabledChangedListener");
    };
};

class TestDisplayInfoChangedListener : public IDisplayInfoChangedListener {
public:
    void OnDisplayInfoChange(const sptr<IRemoteObject>& token,
                             DisplayId displayId,
                             float density,
                             DisplayOrientation orientation) override
    {
        TLOGI(WmsLogTag::DMS, "TestDisplayInfoChangedListener");
    }
};

class TestVisibleWindowNumChangedListener : public IVisibleWindowNumChangedListener {
public:
    void OnVisibleWindowNumChange(const std::vector<VisibleWindowNumInfo>& visibleWindowNumInfo) override
    {
        for (const auto& num : visibleWindowNumInfo) {
            GTEST_LOG_(INFO) << "displayId " << num.displayId << ", visibleWindowNum " << num.visibleWindowNum;
        }
    };
};

class TestDrawingContentChangedListener : public IDrawingContentChangedListener {
public:
    void OnWindowDrawingContentChanged(const std::vector<sptr<WindowDrawingContentInfo>>& windowDrawingInfo)
    {
        TLOGI(WmsLogTag::DMS, "TestDrawingContentChangedListener");
    }
};

class TestFocusChangedListener : public IFocusChangedListener {
public:
    void OnFocused(const sptr<FocusChangeInfo>& focusChangeInfo)
    {
        TLOGI(WmsLogTag::DMS, "TestFocusChangedListener OnFocused()");
    }
    void OnUnfocused(const sptr<FocusChangeInfo>& focusChangeInfo)
    {
        TLOGI(WmsLogTag::DMS, "TestFocusChangedListener OnUnfocused()");
    }
};

class TestWindowStyleChangedListener : public IWindowStyleChangedListener {
public:
    void OnWindowStyleUpdate(WindowStyleType styleType)
    {
        TLOGI(WmsLogTag::DMS, "TestWindowStyleChangedListener");
    }
};

class TestWindowPidVisibilityChangedListener : public IWindowPidVisibilityChangedListener {
public:
    void NotifyWindowPidVisibilityChanged(const sptr<WindowPidVisibilityInfo>& info)
    {
        TLOGI(WmsLogTag::DMS, "TestWindowPidVisibilityChangedListener");
    }
};

class TestIWindowLifeCycleListener : public IWindowLifeCycleListener {
public:
    void OnWindowDestroyed(const WindowLifeCycleInfo& lifeCycleInfo, void* jsWindowNapiValue) override
    {
        listenerLifeCycleInfo.windowId = lifeCycleInfo.windowId;
        listenerLifeCycleInfo.windowType = lifeCycleInfo.windowType;
        listenerLifeCycleInfo.windowName = lifeCycleInfo.windowName;
    }

    TestIWindowLifeCycleListener()
    {
        listenerLifeCycleInfo.windowId = 0;
        listenerLifeCycleInfo.windowType = WindowType::SYSTEM_WINDOW_END;
        listenerLifeCycleInfo.windowName = "";
    }

    WindowLifeCycleInfo listenerLifeCycleInfo;
};

class TestIWindowSupportRotationListener : public IWindowSupportRotationListener {
    public:
    void OnSupportRotationChange(const SupportRotationInfo& supportRotationInfo)
    {
        listenerSupportRotationInfo.displayId_ = supportRotationInfo.displayId_;
        listenerSupportRotationInfo.persistentId_ = supportRotationInfo.persistentId_;
        listenerSupportRotationInfo.containerSupportRotation_ = supportRotationInfo.containerSupportRotation_;
        listenerSupportRotationInfo.sceneSupportRotation_ = supportRotationInfo.sceneSupportRotation_;
        listenerSupportRotationInfo.supportRotationChangeReason_ = supportRotationInfo.supportRotationChangeReason_;
    }

    TestIWindowSupportRotationListener()
    {
        listenerSupportRotationInfo.displayId_ = 0;
        listenerSupportRotationInfo.persistentId_ = 0;
        listenerSupportRotationInfo.containerSupportRotation_ = {false, false, false, false};
        listenerSupportRotationInfo.sceneSupportRotation_ = {false, false, false, false};
        listenerSupportRotationInfo.supportRotationChangeReason_ = "test";
    }

    SupportRotationInfo  listenerSupportRotationInfo;
};

class TestWMSListener : public IWMSConnectionChangedListener {
public:
    void OnConnected(int32_t userId, int32_t screenId) override {}
    void OnDisconnected(int32_t userId, int32_t screenId) override {}
};

class TestInterestWindowIdsListener : public IWindowInfoChangedListener {
public:
    void OnWindowInfoChanged(const WindowInfoList& windowInfoList) override
    {
        received_ = windowInfoList;
    }
    WindowInfoList received_;
};

class WindowManagerTest : public testing::Test {
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

void WindowManagerTest::SetUpTestCase() {}

void WindowManagerTest::TearDownTestCase() {}

void WindowManagerTest::SetUp()
{
    instance_ = &WindowManager::GetInstance(userId_);
    windowAdapter = &WindowAdapter::GetInstance(userId_);

    // Note: To mock WindowAdapter, we can mock WindowAdapter by create a mocked adapter instance and
    // insert it to windowAdapterMap_. Then, WindowAdapter::GetInstance(mockUserId_) will get it.
    mockAdapter_ = sptr<MockWindowAdapter>::MakeSptr(mockUserId_);
    WindowAdapter::windowAdapterMap_[mockUserId_] = mockAdapter_;

    mockInstance_ = &WindowManager::GetInstance(mockUserId_);
}

void WindowManagerTest::TearDown()
{
    WindowManager::RemoveInstanceByUserId(userId_);
    WindowManager::RemoveInstanceByUserId(mockUserId_);

    WindowAdapter::windowAdapterMap_.clear();
}

namespace {
/**
 * @tc.name: Create01
 * @tc.desc: Create window with no WindowName and no abilityToken
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetVisibilityWindowInfo01, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    std::vector<sptr<WindowVisibilityInfo>> infos;
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    auto tempProxy = windowAdapter->windowManagerServiceProxy_;
    if (windowAdapter->windowManagerServiceProxy_ != nullptr) {
        windowAdapter->windowManagerServiceProxy_ = nullptr;
    }
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, instance_->GetVisibilityWindowInfo(infos));
    windowAdapter->windowManagerServiceProxy_ = tempProxy;
}

/**
 * @tc.name: ToggleShownStateForAllAppWindows
 * @tc.desc: ToggleShownStateForAllAppWindows ok
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, ToggleShownStateForAllAppWindows, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    instance_->ToggleShownStateForAllAppWindows();
}

/**
 * @tc.name: Create01
 * @tc.desc: Create window with no WindowName and no abilityToken
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetAccessibilityWindowInfo01, TestSize.Level1)
{
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    infos.clear();
    ASSERT_NE(nullptr, instance_);
    instance_->GetAccessibilityWindowInfo(infos);
}

/**
 * @tc.name: ConvertToRelativeCoordinateExtended
 * @tc.desc: ConvertToRelativeCoordinateExtended ok
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, ConvertToRelativeCoordinateExtended, TestSize.Level1)
{
    Rect rect;
    Rect newRect;
    DisplayId newDisplayId = 0;
    rect = { 100, 2000, 400, 600 };
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    auto tempProxy = windowAdapter->windowManagerServiceProxy_;
    if (windowAdapter->windowManagerServiceProxy_ != nullptr) {
        windowAdapter->windowManagerServiceProxy_ = nullptr;
    }
    auto ret = instance_->ConvertToRelativeCoordinateExtended(rect, newRect, newDisplayId);
    EXPECT_EQ(WMError::WM_DO_NOTHING, ret);
}

/**
 * @tc.name: GetUnreliableWindowInfo
 * @tc.desc: GetUnreliableWindowInfo ok
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetUnreliableWindowInfo, TestSize.Level1)
{
    int32_t windowId = 0;
    std::vector<sptr<UnreliableWindowInfo>> infos;
    ASSERT_NE(nullptr, instance_);
    instance_->GetUnreliableWindowInfo(windowId, infos);
}

/**
 * @tc.name: GetSnapshotByWindowId01
 * @tc.desc: Check GetSnapshotByWindowId01
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetSnapshotByWindowId01, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    int32_t windowId = -1;
    std::shared_ptr<Media::PixelMap> pixelMap = nullptr;
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    auto tempProxy = windowAdapter->windowManagerServiceProxy_;
    if (windowAdapter->windowManagerServiceProxy_ != nullptr) {
        windowAdapter->windowManagerServiceProxy_ = nullptr;
    }
    WMError ret = instance_->GetSnapshotByWindowId(windowId, pixelMap);
    if (SceneBoardJudgement::IsSceneBoardEnabled()) {
        ASSERT_EQ(WMError::WM_ERROR_IPC_FAILED, ret);
    } else {
        ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    }
    windowAdapter->windowManagerServiceProxy_ = tempProxy;
}

/**
 * @tc.name: NotifyScreenshotEvent01
 * @tc.desc: Check NotifyScreenshotEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyScreenshotEvent01, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    auto tempProxy = windowAdapter->windowManagerServiceProxy_;
    if (windowAdapter->windowManagerServiceProxy_ != nullptr) {
        windowAdapter->windowManagerServiceProxy_ = nullptr;
    }
    ScreenshotEventType type = ScreenshotEventType::SCROLL_SHOT_START;
    WMError ret = instance_->NotifyScreenshotEvent(type);
    EXPECT_EQ(ret, WMError::WM_ERROR_SAMGR);
    windowAdapter->windowManagerServiceProxy_ = tempProxy;
}

/**
 * @tc.name: RegisterCameraFloatWindowChangedListener
 * @tc.desc: check RegisterCameraFloatWindowChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterCameraFloatWindowChangedListener, TestSize.Level1)
{
    WMError ret;
    sptr<ICameraFloatWindowChangedListener> listener = nullptr;

    // branch 1
    ret = instance_->RegisterCameraFloatWindowChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: return error
    listener = sptr<TestCameraFloatWindowChangedListener>::MakeSptr();
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->RegisterCameraFloatWindowChangedListener(listener);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 3: use mock and return ok
    ret = mockInstance_->RegisterCameraFloatWindowChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 4: override
    ret = mockInstance_->RegisterCameraFloatWindowChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterCameraFloatWindowChangedListener01
 * @tc.desc: check UnregisterCameraFloatWindowChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterCameraFloatWindowChangedListener01, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->cameraFloatWindowChangedListenerAgent_;
    auto oldListeners = instance_->pImpl_->cameraFloatWindowChangedListeners_;
    instance_->pImpl_->cameraFloatWindowChangedListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    instance_->pImpl_->cameraFloatWindowChangedListeners_.clear();

    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterCameraFloatWindowChangedListener(nullptr));

    sptr<TestCameraFloatWindowChangedListener> listener1 = sptr<TestCameraFloatWindowChangedListener>::MakeSptr();
    sptr<TestCameraFloatWindowChangedListener> listener2 = sptr<TestCameraFloatWindowChangedListener>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterCameraFloatWindowChangedListener(listener1));

    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;

    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_TYPE_FOCUS;
    instance_->pImpl_->cameraFloatWindowChangedListeners_.clear();
    ASSERT_EQ(0, instance_->pImpl_->cameraFloatWindowChangedListeners_.size());
    auto ret = windowAdapter->RegisterWindowManagerAgent(type, nullptr);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret);

    instance_->RegisterCameraFloatWindowChangedListener(listener1);
    instance_->RegisterCameraFloatWindowChangedListener(listener2);
    ASSERT_EQ(0, instance_->pImpl_->cameraFloatWindowChangedListeners_.size());
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterCameraFloatWindowChangedListener(listener1));

    instance_->UnregisterCameraFloatWindowChangedListener(listener2);
    ASSERT_EQ(0, instance_->pImpl_->cameraFloatWindowChangedListeners_.size());
    ASSERT_EQ(nullptr, instance_->pImpl_->cameraFloatWindowChangedListenerAgent_);

    instance_->pImpl_->cameraFloatWindowChangedListeners_.emplace_back(listener1);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterCameraFloatWindowChangedListener(listener1));
    ASSERT_EQ(0, instance_->pImpl_->cameraFloatWindowChangedListeners_.size());

    instance_->pImpl_->cameraFloatWindowChangedListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->cameraFloatWindowChangedListeners_ = oldListeners;
}

/**
 * @tc.name: RegisterVisibilityChangedListener
 * @tc.desc: check RegisterVisibilityChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterVisibilityChangedListener, TestSize.Level1)
{
    WMError ret;
    sptr<IVisibilityChangedListener> listener = nullptr;

    // branch 1
    ret = instance_->RegisterVisibilityChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: return ok
    listener = sptr<TestVisibilityChangedListener>::MakeSptr();
    windowAdapter->isProxyValid_ = true; // skip iniSSMProxy
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->RegisterVisibilityChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 3: return failed due to send ipc failed.
    windowAdapter->isProxyValid_ = false;
    windowAdapter->InitSSMProxy();
    ASSERT_NE(nullptr, windowAdapter->windowManagerServiceProxy_);

    ret = instance_->RegisterVisibilityChangedListener(listener);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 4: return ok, mock the
    // WindowAdapter::GetInstance(userId_).RegisterWindowManagerAgent() return ok
    ret = mockInstance_->RegisterVisibilityChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 5: branch overried
    ret = mockInstance_->RegisterVisibilityChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: RegisterWindowUpdateListener01
 * @tc.desc: check RegisterWindowUpdateListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterWindowUpdateListener01, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->windowUpdateListenerAgent_;
    auto oldListeners = instance_->pImpl_->windowUpdateListeners_;
    instance_->pImpl_->windowUpdateListenerAgent_ = nullptr;
    instance_->pImpl_->windowUpdateListeners_.clear();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->RegisterWindowUpdateListener(nullptr));

    sptr<TestWindowUpdateListener> listener = sptr<TestWindowUpdateListener>::MakeSptr();
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    
    instance_->RegisterWindowUpdateListener(listener);
    ASSERT_EQ(0, instance_->pImpl_->windowUpdateListeners_.size());

    // to check that the same listner can not be registered twice
    instance_->RegisterWindowUpdateListener(listener);
    ASSERT_EQ(0, instance_->pImpl_->windowUpdateListeners_.size());

    instance_->pImpl_->windowUpdateListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->windowUpdateListeners_ = oldListeners;
}

/**
 * @tc.name: UnregisterWindowUpdateListener01
 * @tc.desc: check UnregisterWindowUpdateListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterWindowUpdateListener01, TestSize.Level1)
{
    auto oldWindowManagerAgent = instance_->pImpl_->windowUpdateListenerAgent_;
    auto oldListeners = instance_->pImpl_->windowUpdateListeners_;
    instance_->pImpl_->windowUpdateListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    instance_->pImpl_->windowUpdateListeners_.clear();

    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterWindowUpdateListener(nullptr));

    sptr<TestWindowUpdateListener> listener1 = sptr<TestWindowUpdateListener>::MakeSptr();
    sptr<TestWindowUpdateListener> listener2 = sptr<TestWindowUpdateListener>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowUpdateListener(listener1));

    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;

    instance_->RegisterWindowUpdateListener(listener1);
    instance_->RegisterWindowUpdateListener(listener2);
    ASSERT_EQ(0, instance_->pImpl_->windowUpdateListeners_.size());

    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowUpdateListener(listener1));
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowUpdateListener(listener2));
    ASSERT_EQ(0, instance_->pImpl_->windowUpdateListeners_.size());
    ASSERT_EQ(nullptr, instance_->pImpl_->windowUpdateListenerAgent_);

    instance_->pImpl_->windowUpdateListeners_.emplace_back(listener1);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowUpdateListener(listener1));
    ASSERT_EQ(0, instance_->pImpl_->windowUpdateListeners_.size());

    instance_->pImpl_->windowUpdateListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->windowUpdateListeners_ = oldListeners;
}

/**
 * @tc.name: UnregisterWindowModeChangedListener01
 * @tc.desc: check UnregisterWindowModeChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterWindowModeChangedListener01, TestSize.Level1)
{
    auto oldWindowManagerAgent = instance_->pImpl_->windowModeListenerAgent_;
    auto oldListeners = instance_->pImpl_->windowModeListeners_;
    instance_->pImpl_->windowModeListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    instance_->pImpl_->windowModeListeners_.clear();

    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterWindowModeChangedListener(nullptr));

    sptr<TestWindowModeChangedListener> listener1 = sptr<TestWindowModeChangedListener>::MakeSptr();
    sptr<TestWindowModeChangedListener> listener2 = sptr<TestWindowModeChangedListener>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowModeChangedListener(listener1));

    instance_->RegisterWindowModeChangedListener(listener1);
    instance_->RegisterWindowModeChangedListener(listener2);

    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowModeChangedListener(listener1));
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowModeChangedListener(listener2));
    ASSERT_EQ(nullptr, instance_->pImpl_->windowModeListenerAgent_);

    instance_->pImpl_->windowModeListeners_.emplace_back(listener1);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWindowModeChangedListener(listener1));
    ASSERT_EQ(0, instance_->pImpl_->windowModeListeners_.size());

    instance_->pImpl_->windowModeListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->windowModeListeners_ = oldListeners;
}

/**
 * @tc.name: RegisterSystemBarChangedListener01
 * @tc.desc: check RegisterSystemBarChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterSystemBarChangedListener01, TestSize.Level1)
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
HWTEST_F(WindowManagerTest, UnregisterSystemBarChangedListener01, TestSize.Level1)
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

/**
 * @tc.name: RegisterWaterMarkListener01
 * @tc.desc: check RegisterWaterMarkListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterWaterMarkFlagChangedListener01, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    instance_->pImpl_->waterMarkFlagChangeAgent_ = nullptr;
    instance_->pImpl_->waterMarkFlagChangeListeners_.clear();

    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->RegisterWaterMarkFlagChangedListener(nullptr));

    auto listener = sptr<TestWaterMarkFlagChangeListener>::MakeSptr();
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    instance_->RegisterWaterMarkFlagChangedListener(listener);
    ASSERT_EQ(0, instance_->pImpl_->waterMarkFlagChangeListeners_.size());

    // to check that the same listner can not be registered twice
}

/**
 * @tc.name: UnregisterWaterMarkFlagChangedListener01
 * @tc.desc: check UnregisterWaterMarkFlagChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterWaterMarkFlagChangedListener01, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    instance_->pImpl_->waterMarkFlagChangeAgent_ = nullptr;
    instance_->pImpl_->waterMarkFlagChangeListeners_.clear();

    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterWaterMarkFlagChangedListener(nullptr));

    sptr<TestWaterMarkFlagChangeListener> listener1 = sptr<TestWaterMarkFlagChangeListener>::MakeSptr();
    sptr<TestWaterMarkFlagChangeListener> listener2 = sptr<TestWaterMarkFlagChangeListener>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWaterMarkFlagChangedListener(listener1));

    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    instance_->RegisterWaterMarkFlagChangedListener(listener1);
    instance_->RegisterWaterMarkFlagChangedListener(listener2);
    ASSERT_EQ(0, instance_->pImpl_->waterMarkFlagChangeListeners_.size());

    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWaterMarkFlagChangedListener(listener1));
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWaterMarkFlagChangedListener(listener2));
    ASSERT_EQ(0, instance_->pImpl_->waterMarkFlagChangeListeners_.size());

    // if agent == nullptr, it can not be crashed.
    instance_->pImpl_->waterMarkFlagChangeListeners_.push_back(listener1);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterWaterMarkFlagChangedListener(listener1));
    ASSERT_EQ(0, instance_->pImpl_->waterMarkFlagChangeListeners_.size());
}

/**
 * @tc.name: RegisterGestureNavigationEnabledChangedListener
 * @tc.desc: check RegisterGestureNavigationEnabledChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterGestureNavigationEnabledChangedListener, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    instance_->pImpl_->gestureNavigationEnabledAgent_ = nullptr;
    instance_->pImpl_->gestureNavigationEnabledListeners_.clear();

    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->RegisterGestureNavigationEnabledChangedListener(nullptr));

    sptr<TestGestureNavigationEnabledChangedListener> listener =
        sptr<TestGestureNavigationEnabledChangedListener>::MakeSptr();

    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    instance_->RegisterGestureNavigationEnabledChangedListener(listener);
    ASSERT_EQ(0, instance_->pImpl_->gestureNavigationEnabledListeners_.size());

    // to check that the same listner can not be registered twice
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, instance_->RegisterGestureNavigationEnabledChangedListener(listener));
    ASSERT_EQ(0, instance_->pImpl_->gestureNavigationEnabledListeners_.size());
}

/**
 * @tc.name: UnregisterGestureNavigationEnabledChangedListener
 * @tc.desc: check UnregisterGestureNavigationEnabledChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterGestureNavigationEnabledChangedListener, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    instance_->pImpl_->gestureNavigationEnabledAgent_ = nullptr;
    instance_->pImpl_->gestureNavigationEnabledListeners_.clear();

    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterGestureNavigationEnabledChangedListener(nullptr));

    sptr<TestGestureNavigationEnabledChangedListener> listener1 =
        sptr<TestGestureNavigationEnabledChangedListener>::MakeSptr();
    sptr<TestGestureNavigationEnabledChangedListener> listener2 =
        sptr<TestGestureNavigationEnabledChangedListener>::MakeSptr();
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM,
              instance_->UnregisterGestureNavigationEnabledChangedListener(listener1));
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    instance_->RegisterGestureNavigationEnabledChangedListener(listener1);
    instance_->RegisterGestureNavigationEnabledChangedListener(listener2);
    ASSERT_EQ(0, instance_->pImpl_->gestureNavigationEnabledListeners_.size());

    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, instance_->UnregisterGestureNavigationEnabledChangedListener(listener1));
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, instance_->UnregisterGestureNavigationEnabledChangedListener(listener2));
    ASSERT_EQ(0, instance_->pImpl_->gestureNavigationEnabledListeners_.size());
    ASSERT_EQ(nullptr, instance_->pImpl_->gestureNavigationEnabledAgent_);

    // if agent == nullptr, it can not be crashed.
    instance_->pImpl_->gestureNavigationEnabledListeners_.push_back(listener1);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterGestureNavigationEnabledChangedListener(listener1));
    ASSERT_EQ(0, instance_->pImpl_->gestureNavigationEnabledListeners_.size());
}

/**
 * @tc.name: GetUIContentRemoteObj
 * @tc.desc: GetUIContentRemoteObj
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetUIContentRemoteObj, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    sptr<IRemoteObject> remoteObj;
    WMError res = instance_->GetUIContentRemoteObj(1, remoteObj);
    ASSERT_EQ(res, WMError::WM_ERROR_SAMGR);
}

/**
 * @tc.name: GetRootUIContentRemoteObj
 * @tc.desc: test GetRootUIContentRemoteObj rpc is ok
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetRootUIContentRemoteObj, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    DisplayId displayId = 0;
    sptr<IRemoteObject> uiContentRemoteObj = nullptr;
    auto expectRet = WindowAdapter::GetInstance(userId_).GetRootUIContentRemoteObj(displayId, uiContentRemoteObj);
    auto ret = instance_->GetRootUIContentRemoteObj(displayId, uiContentRemoteObj);
    EXPECT_EQ(ret, expectRet);
}

/**
 * @tc.name: GetFocusWindowInfo
 * @tc.desc: window GetFocusWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetFocusWindowInfo, TestSize.Level1)
{
    FocusChangeInfo focusInfo;
    DisplayId displayId = 0;
    ASSERT_NE(nullptr, instance_);
    instance_->GetFocusWindowInfo(focusInfo, displayId);
}

/**
 * @tc.name: MinimizeAllAppWindows
 * @tc.desc: window MinimizeAllAppWindows
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, MinimizeAllAppWindows, TestSize.Level1)
{
    DisplayId displayId = 0;
    ASSERT_NE(nullptr, instance_);
    instance_->MinimizeAllAppWindows(displayId);
}

/**
 * @tc.name: SetWindowLayoutMode
 * @tc.desc: window SetWindowLayoutMode
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, SetWindowLayoutMode, TestSize.Level1)
{
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    WMError ret = instance_->SetWindowLayoutMode(WindowLayoutMode::BASE);
    ASSERT_EQ(ret, WMError::WM_ERROR_SAMGR);
}

/**
 * @tc.name: SkipSnapshotForAppProcess
 * @tc.desc: check SkipSnapshotForAppProcess
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, SkipSnapshotForAppProcess, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    int32_t pid = 1000;
    bool skip = true;
    auto ret = instance_->SkipSnapshotForAppProcess(pid, skip);
    ASSERT_EQ(WMError::WM_DO_NOTHING, ret);
}

/**
 * @tc.name: UpdateCameraFloatWindowStatus
 * @tc.desc: UpdateCameraFloatWindowStatus
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UpdateCameraFloatWindowStatus, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    uint32_t accessTokenId = 0;
    bool isShowing = true;
    WindowManager::GetInstance().UpdateCameraFloatWindowStatus(accessTokenId, isShowing);
    EXPECT_FALSE(g_errLog.find("Camera float window, accessTokenId=%{private}u, isShowing=%{public}u")
        != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: NotifyWaterMarkFlagChangedResult
 * @tc.desc: NotifyWaterMarkFlagChangedResult
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyWaterMarkFlagChangedResult, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    bool showwatermark = true;
    WindowManager::GetInstance().NotifyWaterMarkFlagChangedResult(showwatermark);
    EXPECT_FALSE(g_errLog.find("Camera float window, accessTokenId=%{private}u, isShowing=%{public}u")
        != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: NotifyGestureNavigationEnabledResult
 * @tc.desc: NotifyGestureNavigationEnabledResult
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyGestureNavigationEnabledResult, TestSize.Level1)
{
    g_errLog.clear();
    LOG_SetCallback(MyLogCallback);
    bool enable = true;
    WindowManager::GetInstance().NotifyGestureNavigationEnabledResult(enable);
    EXPECT_FALSE(g_errLog.find("Notify gesture navigation enable result, enable=%{public}d")
        != std::string::npos);
    LOG_SetCallback(nullptr);
}

/**
 * @tc.name: DumpSessionAll
 * @tc.desc: DumpSessionAll
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, DumpSessionAll, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    ASSERT_NE(windowAdapter, nullptr);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    std::vector<std::string> infos;
    infos.push_back("DumpSessionWithId");
    WMError res = instance_->DumpSessionAll(infos);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, res);
}

/**
 * @tc.name: DumpSessionWithId
 * @tc.desc: DumpSessionWithId
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, DumpSessionWithId, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    std::vector<std::string> infos;
    infos.push_back("DumpSessionWithId");
    int32_t persistentId = 0;
    WMError res = instance_->DumpSessionWithId(persistentId, infos);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, res);
}

/**
 * @tc.name: GetWindowModeType01
 * @tc.desc: GetWindowModeType01
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetWindowModeType01, TestSize.Level1)
{
    std::vector<sptr<AccessibilityWindowInfo>> infos;
    infos.clear();
    WindowModeType windowModeType;
    ASSERT_NE(nullptr, instance_);
    instance_->GetWindowModeType(windowModeType);
}

/**
 * @tc.name: RegisterVisibleWindowNumChangedListener
 * @tc.desc: check RegisterVisibleWindowNumChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterVisibleWindowNumChangedListener, TestSize.Level1)
{
    instance_->pImpl_->visibleWindowNumChangedListenerAgent_ = nullptr;
    instance_->pImpl_->visibleWindowNumChangedListeners_.clear();

    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->RegisterVisibleWindowNumChangedListener(nullptr));

    sptr<TestVisibleWindowNumChangedListener> listener = sptr<TestVisibleWindowNumChangedListener>::MakeSptr();
    ASSERT_NE(nullptr, instance_);
    instance_->RegisterVisibleWindowNumChangedListener(listener);
}

/**
 * @tc.name: UnregisterVisibleWindowNumChangedListener
 * @tc.desc: check UnregisterVisibleWindowNumChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterVisibleWindowNumChangedListener, TestSize.Level1)
{
    instance_->pImpl_->visibleWindowNumChangedListenerAgent_ = nullptr;
    instance_->pImpl_->visibleWindowNumChangedListeners_.clear();

    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterVisibleWindowNumChangedListener(nullptr));

    sptr<TestVisibleWindowNumChangedListener> listener1 = sptr<TestVisibleWindowNumChangedListener>::MakeSptr();
    sptr<TestVisibleWindowNumChangedListener> listener2 = sptr<TestVisibleWindowNumChangedListener>::MakeSptr();

    instance_->RegisterVisibleWindowNumChangedListener(listener1);
    instance_->RegisterVisibleWindowNumChangedListener(listener2);

    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterVisibleWindowNumChangedListener(listener1));
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterVisibleWindowNumChangedListener(listener2));

    // if agent == nullptr, it can not be crashed.
    instance_->pImpl_->visibleWindowNumChangedListeners_.push_back(listener1);
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterVisibleWindowNumChangedListener(listener1));
}

/**
 * @tc.name: RegisterAndOnVisibleWindowNumChanged
 * @tc.desc: check RegisterAndOnVisibleWindowNumChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterAndOnVisibleWindowNumChanged, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    instance_->pImpl_->visibleWindowNumChangedListenerAgent_ = nullptr;
    instance_->pImpl_->visibleWindowNumChangedListeners_.clear();

    sptr<TestVisibleWindowNumChangedListener> listener = sptr<TestVisibleWindowNumChangedListener>::MakeSptr();
    instance_->RegisterVisibleWindowNumChangedListener(listener);

    std::vector<VisibleWindowNumInfo> visibleWindowNumInfo;
    VisibleWindowNumInfo newInfo;
    newInfo.displayId = 0;
    newInfo.visibleWindowNum = 2;
    visibleWindowNumInfo.push_back(newInfo);
    instance_->UpdateVisibleWindowNum(visibleWindowNumInfo);
}

/**
 * @tc.name: Test01
 * @tc.desc: Test01
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, Test01, TestSize.Level1)
{
    sptr<IWMSConnectionChangedListener> listener = nullptr;
    WMError res = WindowManager::GetInstance().RegisterWMSConnectionChangedListener(listener);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, res);
    WMError res1 = WindowManager::GetInstance().UnregisterWMSConnectionChangedListener();
    ASSERT_EQ(WMError::WM_OK, res1);
    WindowManager::GetInstance().RaiseWindowToTop(5);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    WMError res3 = instance_->NotifyWindowExtensionVisibilityChange(5, 5, true);
    ASSERT_EQ(WMError::WM_DO_NOTHING, res3);
    WindowManager::GetInstance().ShiftAppWindowFocus(0, 1);
}

/**
 * @tc.name: RegisterDisplayInfoChangedListener
 * @tc.desc: check RegisterDisplayInfoChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterDisplayInfoChangedListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    instance_->pImpl_->displayInfoChangedListeners_.clear();

    sptr<IRemoteObject> targetToken = nullptr;
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->RegisterDisplayInfoChangedListener(targetToken, nullptr));

    targetToken = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->RegisterDisplayInfoChangedListener(targetToken, nullptr));

    sptr<IDisplayInfoChangedListener> listener = sptr<TestDisplayInfoChangedListener>::MakeSptr();
    sptr<IDisplayInfoChangedListener> listener2 = sptr<TestDisplayInfoChangedListener>::MakeSptr();

    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->RegisterDisplayInfoChangedListener(nullptr, listener));
    ASSERT_EQ(0, instance_->pImpl_->displayInfoChangedListeners_.size());
    ASSERT_EQ(WMError::WM_OK, instance_->RegisterDisplayInfoChangedListener(targetToken, listener));
    ASSERT_EQ(1, instance_->pImpl_->displayInfoChangedListeners_.size());

    // to check that the same listner can not be registered twice
    ASSERT_EQ(WMError::WM_OK, instance_->RegisterDisplayInfoChangedListener(targetToken, listener));
    ASSERT_EQ(1, instance_->pImpl_->displayInfoChangedListeners_.size());

    ASSERT_EQ(WMError::WM_OK, instance_->RegisterDisplayInfoChangedListener(targetToken, listener2));
    ASSERT_EQ(1, instance_->pImpl_->displayInfoChangedListeners_.size());
    auto iter = instance_->pImpl_->displayInfoChangedListeners_.find(targetToken);
    ASSERT_NE(instance_->pImpl_->displayInfoChangedListeners_.end(), iter);
    ASSERT_EQ(2, iter->second.size());
}

/**
 * @tc.name: UnregisterDisplayInfoChangedListener
 * @tc.desc: check UnregisterDisplayInfoChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterDisplayInfoChangedListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    instance_->pImpl_->displayInfoChangedListeners_.clear();

    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterDisplayInfoChangedListener(nullptr, nullptr));

    sptr<IRemoteObject> targetToken = sptr<IRemoteObjectMocker>::MakeSptr();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterDisplayInfoChangedListener(targetToken, nullptr));

    sptr<IDisplayInfoChangedListener> listener = sptr<TestDisplayInfoChangedListener>::MakeSptr();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterDisplayInfoChangedListener(nullptr, listener));

    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterDisplayInfoChangedListener(targetToken, listener));

    sptr<IRemoteObject> targetToken2 = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<IDisplayInfoChangedListener> listener2 = sptr<TestDisplayInfoChangedListener>::MakeSptr();

    // the same token can have multiple listeners
    ASSERT_EQ(WMError::WM_OK, instance_->RegisterDisplayInfoChangedListener(targetToken, listener));
    ASSERT_EQ(1, instance_->pImpl_->displayInfoChangedListeners_.size());
    ASSERT_EQ(WMError::WM_OK, instance_->RegisterDisplayInfoChangedListener(targetToken, listener2));
    ASSERT_EQ(1, instance_->pImpl_->displayInfoChangedListeners_.size());
    auto iter = instance_->pImpl_->displayInfoChangedListeners_.find(targetToken);
    ASSERT_NE(instance_->pImpl_->displayInfoChangedListeners_.end(), iter);
    ASSERT_EQ(2, iter->second.size());

    ASSERT_EQ(WMError::WM_OK, instance_->RegisterDisplayInfoChangedListener(targetToken2, listener));
    ASSERT_EQ(2, instance_->pImpl_->displayInfoChangedListeners_.size());

    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterDisplayInfoChangedListener(targetToken, listener));
    ASSERT_EQ(1, iter->second.size());
    ASSERT_EQ(2, instance_->pImpl_->displayInfoChangedListeners_.size());

    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterDisplayInfoChangedListener(targetToken, listener2));
    ASSERT_EQ(1, instance_->pImpl_->displayInfoChangedListeners_.size());
    ASSERT_EQ(WMError::WM_OK, instance_->UnregisterDisplayInfoChangedListener(targetToken2, listener));
    ASSERT_EQ(0, instance_->pImpl_->displayInfoChangedListeners_.size());
}

/**
 * @tc.name: NotifyDisplayInfoChanged
 * @tc.desc: check NotifyDisplayInfoChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyDisplayInfoChanged, TestSize.Level1)
{
    sptr<IRemoteObject> targetToken = sptr<IRemoteObjectMocker>::MakeSptr();
    DisplayId displayId = 0;
    float density = 0.2f;
    DisplayOrientation orientation = DisplayOrientation::UNKNOWN;

    instance_->pImpl_->displayInfoChangedListeners_.clear();
    instance_->pImpl_->NotifyDisplayInfoChanged(targetToken, displayId, density, orientation);

    sptr<IRemoteObject> targetToken2 = sptr<IRemoteObjectMocker>::MakeSptr();
    sptr<IDisplayInfoChangedListener> listener = sptr<TestDisplayInfoChangedListener>::MakeSptr();

    ASSERT_EQ(WMError::WM_OK, instance_->RegisterDisplayInfoChangedListener(targetToken, listener));
    ASSERT_EQ(1, instance_->pImpl_->displayInfoChangedListeners_.size());
    instance_->pImpl_->NotifyDisplayInfoChanged(targetToken, displayId, density, orientation);

    ASSERT_EQ(WMError::WM_OK, instance_->RegisterDisplayInfoChangedListener(targetToken2, listener));
    ASSERT_EQ(2, instance_->pImpl_->displayInfoChangedListeners_.size());
    instance_->pImpl_->NotifyDisplayInfoChanged(targetToken, displayId, density, orientation);
    instance_->pImpl_->NotifyDisplayInfoChanged(targetToken2, displayId, density, orientation);
    // no repeated notification is sent if parameters do not change
    instance_->pImpl_->NotifyDisplayInfoChanged(targetToken, displayId, density, orientation);
}

/**
 * @tc.name: RegisterWindowStyleChangedListener
 * @tc.desc: check RegisterWindowStyleChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterWindowStyleChangedListener, TestSize.Level1)
{
    auto oldWindowManagerAgent = instance_->pImpl_->windowStyleListenerAgent_;
    auto oldListeners = instance_->pImpl_->windowStyleListeners_;
    instance_->pImpl_->windowStyleListenerAgent_ = nullptr;
    instance_->pImpl_->windowStyleListeners_.clear();
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, instance_->RegisterWindowStyleChangedListener(nullptr));
}

/**
 * @tc.name: UnregisterWindowStyleChangedListener
 * @tc.desc: check UnregisterWindowStyleChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterWindowStyleChangedListener, TestSize.Level1)
{
    auto& windowManager = WindowManager::GetInstance();
    auto oldWindowManagerAgent = windowManager.pImpl_->windowStyleListenerAgent_;
    auto oldListeners = windowManager.pImpl_->windowStyleListeners_;
    windowManager.pImpl_->windowStyleListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    windowManager.pImpl_->windowStyleListeners_.clear();
    // check nullpter
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, windowManager.UnregisterWindowStyleChangedListener(nullptr));

    sptr<TestWindowStyleChangedListener> listener1 = sptr<TestWindowStyleChangedListener>::MakeSptr();
    sptr<TestWindowStyleChangedListener> listener2 = sptr<TestWindowStyleChangedListener>::MakeSptr();
    ASSERT_EQ(WMError::WM_OK, windowManager.UnregisterWindowStyleChangedListener(listener1));
}

/**
 * @tc.name: NotifyWindowStyleChange
 * @tc.desc: check NotifyWindowStyleChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyWindowStyleChange, TestSize.Level1)
{
    WindowStyleType type = Rosen::WindowStyleType::WINDOW_STYLE_DEFAULT;
    auto ret = WindowManager::GetInstance().NotifyWindowStyleChange(type);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: GetWindowStyleType
 * @tc.desc: check GetWindowStyleType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetWindowStyleType, TestSize.Level1)
{
    WindowStyleType type;
    type = WindowManager::GetInstance().GetWindowStyleType();
    ASSERT_EQ(Rosen::WindowStyleType::WINDOW_STYLE_DEFAULT, type);
}

/**
 * @tc.name: ShiftAppWindowFocus01
 * @tc.desc: check ShiftAppWindowFocus
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, ShiftAppWindowFocus01, TestSize.Level1)
{
    WMError ret = WindowManager::GetInstance().ShiftAppWindowFocus(0, 1);
    ASSERT_NE(WMError::WM_OK, ret);
}

/**
 * @tc.name: RegisterVisibleWindowNumChangedListener01
 * @tc.desc: check RegisterVisibleWindowNumChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterVisibleWindowNumChangedListener01, TestSize.Level1)
{
    WMError ret;
    sptr<IVisibleWindowNumChangedListener> listener = sptr<TestVisibleWindowNumChangedListener>::MakeSptr();
    ret = WindowManager::GetInstance().RegisterVisibleWindowNumChangedListener(listener);
    ASSERT_NE(WMError::WM_OK, ret);

    ret = WindowManager::GetInstance().RegisterVisibleWindowNumChangedListener(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: UnregisterVisibleWindowNumChangedListener01
 * @tc.desc: check UnregisterVisibleWindowNumChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterVisibleWindowNumChangedListener01, TestSize.Level1)
{
    WMError ret;
    sptr<IVisibleWindowNumChangedListener> listener = sptr<TestVisibleWindowNumChangedListener>::MakeSptr();
    ret = WindowManager::GetInstance().UnregisterVisibleWindowNumChangedListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);

    ret = WindowManager::GetInstance().UnregisterVisibleWindowNumChangedListener(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: RegisterDrawingContentChangedListener01
 * @tc.desc: check RegisterDrawingContentChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterDrawingContentChangedListener01, TestSize.Level1)
{
    WMError ret;
    sptr<IDrawingContentChangedListener> listener = sptr<TestDrawingContentChangedListener>::MakeSptr();
    ret = WindowManager::GetInstance().RegisterDrawingContentChangedListener(listener);
    ASSERT_NE(WMError::WM_OK, ret);

    ret = WindowManager::GetInstance().RegisterDrawingContentChangedListener(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: UnregisterDrawingContentChangedListener01
 * @tc.desc: check UnregisterDrawingContentChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterDrawingContentChangedListener01, TestSize.Level1)
{
    WMError ret;
    sptr<IDrawingContentChangedListener> listener = sptr<TestDrawingContentChangedListener>::MakeSptr();
    ret = WindowManager::GetInstance().UnregisterDrawingContentChangedListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);

    ret = WindowManager::GetInstance().UnregisterDrawingContentChangedListener(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: RegisterFocusChangedListener
 * @tc.desc: check RegisterFocusChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterFocusChangedListener, TestSize.Level1)
{
    WMError ret;
    sptr<IFocusChangedListener> listener = nullptr;

    // branch 1
    ret = instance_->RegisterFocusChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: return ok
    listener = sptr<TestFocusChangedListener>::MakeSptr();
    windowAdapter->isProxyValid_ = true; // skip iniSSMProxy
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->RegisterFocusChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 3: return failed, not use ActiveFaultAgentReregister
    windowAdapter->isProxyValid_ = false;
    ASSERT_EQ(true, windowAdapter->InitSSMProxy());
    ASSERT_NE(nullptr, windowAdapter->windowManagerServiceProxy_);

    ret = instance_->RegisterFocusChangedListener(listener);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 4: use mock and return ok
    ret = mockInstance_->RegisterFocusChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 5: branch overried
    ret = mockInstance_->RegisterFocusChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterFocusChangedListener01
 * @tc.desc: check UnregisterFocusChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterFocusChangedListener01, TestSize.Level1)
{
    WMError ret;
    sptr<IFocusChangedListener> listener = sptr<TestFocusChangedListener>::MakeSptr();
    ret = WindowManager::GetInstance().UnregisterFocusChangedListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);

    ret = WindowManager::GetInstance().UnregisterFocusChangedListener(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: SetProcessWatermark
 * @tc.desc: check SetProcessWatermark
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, SetProcessWatermark, TestSize.Level1)
{
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    int32_t pid = 1000;
    const std::string watermarkName = "SetProcessWatermarkName";
    bool isEnabled = true;
    auto ret = instance_->SetProcessWatermark(pid, watermarkName, isEnabled);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret);
}

/**
 * @tc.name: NotifyDisplayInfoChange01
 * @tc.desc: check NotifyDisplayInfoChange, Token is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyDisplayInfoChange01, TestSize.Level1)
{
    WMError ret = WindowManager::GetInstance().NotifyDisplayInfoChange(nullptr, 1, 2, DisplayOrientation::PORTRAIT);
    ASSERT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
}

/**
 * @tc.name: NotifyDisplayInfoChange02
 * @tc.desc: check NotifyDisplayInfoChange, Token is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyDisplayInfoChange02, TestSize.Level1)
{
    sptr<IRemoteObject> Token = sptr<IRemoteObjectMocker>::MakeSptr();
    WMError ret = WindowManager::GetInstance().NotifyDisplayInfoChange(Token, 1, 2, DisplayOrientation::PORTRAIT);
    ASSERT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: NotifyWMSDisconnected01
 * @tc.desc: check NotifyWMSDisconnected
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyWMSDisconnected01, TestSize.Level1)
{
    WMError ret = WindowManager::GetInstance().ShiftAppWindowFocus(0, 1);
    ASSERT_NE(WMError::WM_OK, ret);
    WindowManager::GetInstance().pImpl_->NotifyWMSDisconnected(1, 2);
}

/**
 * @tc.name: NotifyFocused01
 * @tc.desc: check NotifyFocused
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyFocused01, TestSize.Level1)
{
    sptr<FocusChangeInfo> focusChangeInfo = sptr<FocusChangeInfo>::MakeSptr();
    ASSERT_NE(focusChangeInfo, nullptr);

    WindowManager::GetInstance().pImpl_->NotifyFocused(focusChangeInfo);
}

/**
 * @tc.name: NotifyUnfocused01
 * @tc.desc: check NotifyUnfocused
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyUnfocused01, TestSize.Level1)
{
    sptr<FocusChangeInfo> focusChangeInfo = sptr<FocusChangeInfo>::MakeSptr();
    ASSERT_NE(focusChangeInfo, nullptr);

    WindowManager::GetInstance().pImpl_->NotifyUnfocused(focusChangeInfo);
}

/**
 * @tc.name: NotifyAccessibilityWindowInfo01
 * @tc.desc: check NotifyAccessibilityWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyAccessibilityWindowInfo01, TestSize.Level1)
{
    WMError ret = WindowManager::GetInstance().ShiftAppWindowFocus(0, 1);
    ASSERT_NE(WMError::WM_OK, ret);
    sptr<AccessibilityWindowInfo> info = sptr<AccessibilityWindowInfo>::MakeSptr();
    ASSERT_NE(info, nullptr);

    std::vector<sptr<AccessibilityWindowInfo>> infos;
    infos.push_back(info);
    WindowManager::GetInstance().pImpl_->NotifyAccessibilityWindowInfo(infos, WindowUpdateType::WINDOW_UPDATE_ACTIVE);

    infos.clear();
    infos.push_back(nullptr);
    WindowManager::GetInstance().pImpl_->NotifyAccessibilityWindowInfo(infos, WindowUpdateType::WINDOW_UPDATE_ACTIVE);
}

/**
 * @tc.name: NotifyVisibleWindowNumChanged01
 * @tc.desc: check NotifyVisibleWindowNumChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyVisibleWindowNumChanged01, TestSize.Level1)
{
    std::vector<VisibleWindowNumInfo> visibleWindowNumInfo;
    WindowManager::GetInstance().pImpl_->visibleWindowNumChangedListeners_.clear();
    WindowManager::GetInstance().pImpl_->visibleWindowNumChangedListeners_.push_back(nullptr);
    WindowManager::GetInstance().pImpl_->NotifyVisibleWindowNumChanged(visibleWindowNumInfo);

    sptr<IVisibleWindowNumChangedListener> listener = sptr<TestVisibleWindowNumChangedListener>::MakeSptr();
    ASSERT_NE(listener, nullptr);
    WindowManager::GetInstance().pImpl_->visibleWindowNumChangedListeners_.clear();
    WindowManager::GetInstance().pImpl_->visibleWindowNumChangedListeners_.push_back(listener);
    WindowManager::GetInstance().pImpl_->NotifyVisibleWindowNumChanged(visibleWindowNumInfo);
}

/**
 * @tc.name: RegisterWindowPidVisibilityChangedListener
 * @tc.desc: check RegisterWindowPidVisibilityChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterWindowPidVisibilityChangedListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    WMError ret;
    sptr<IWindowPidVisibilityChangedListener> listener = sptr<TestWindowPidVisibilityChangedListener>::MakeSptr();
    ASSERT_NE(nullptr, listener);
    ret = instance_->RegisterWindowPidVisibilityChangedListener(listener);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret);

    ret = WindowManager::GetInstance().RegisterWindowPidVisibilityChangedListener(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: UnregisterWindowPidVisibilityChangedListener
 * @tc.desc: check UnregisterWindowPidVisibilityChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterWindowPidVisibilityChangedListener, TestSize.Level1)
{
    WMError ret;
    sptr<IWindowPidVisibilityChangedListener> listener = sptr<TestWindowPidVisibilityChangedListener>::MakeSptr();
    ret = WindowManager::GetInstance().UnregisterWindowPidVisibilityChangedListener(listener);
    ASSERT_EQ(WMError::WM_OK, ret);

    ret = WindowManager::GetInstance().UnregisterWindowPidVisibilityChangedListener(nullptr);
    ASSERT_EQ(WMError::WM_ERROR_NULLPTR, ret);
}

/**
 * @tc.name: NotifyWindowPidVisibilityChanged
 * @tc.desc: NotifyWindowPidVisibilityChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyWindowPidVisibilityChanged, TestSize.Level1)
{
    sptr<WindowPidVisibilityInfo> info = sptr<WindowPidVisibilityInfo>::MakeSptr();
    WindowManager::GetInstance().NotifyWindowPidVisibilityChanged(info);
    ASSERT_NE(info, nullptr);
}

/**
 * @tc.name: UpdateScreenLockStatusForApp
 * @tc.desc: check UpdateScreenLockStatusForApp
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UpdateScreenLockStatusForApp, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    auto ret = instance_->UpdateScreenLockStatusForApp("", true);
    ASSERT_EQ(ret, WMError::WM_DO_NOTHING);
}

/**
 * @tc.name: GetDisplayIdByWindowId
 * @tc.desc: check GetDisplayIdByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetDisplayIdByWindowId, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    const std::vector<uint64_t> windowIds = { 1, 2 };
    std::unordered_map<uint64_t, DisplayId> windowDisplayIdMap;
    auto ret = instance_->GetDisplayIdByWindowId(windowIds, windowDisplayIdMap);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret);
}

/**
 * @tc.name: SetGlobalDragResizeType
 * @tc.desc: check SetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, SetGlobalDragResizeType, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    auto ret = instance_->SetGlobalDragResizeType(dragResizeType);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret);
}

/**
 * @tc.name: GetGlobalDragResizeType
 * @tc.desc: check GetGlobalDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetGlobalDragResizeType, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    auto ret = instance_->GetGlobalDragResizeType(dragResizeType);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret);
}

/**
 * @tc.name: SetAppDragResizeType
 * @tc.desc: check SetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, SetAppDragResizeType, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    DragResizeType dragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    const std::string bundleName = "test";
    auto ret = instance_->SetAppDragResizeType(bundleName, dragResizeType);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret);
}

/**
 * @tc.name: GetAppDragResizeType
 * @tc.desc: check GetAppDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetAppDragResizeType, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    const std::string bundleName = "test";
    auto ret = instance_->GetAppDragResizeType(bundleName, dragResizeType);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret);
}

/**
 * @tc.name: EffectiveDragResizeType
 * @tc.desc: test EffectiveDragResizeType
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, EffectiveDragResizeType, TestSize.Level1)
{
    DragResizeType dragResizeType = DragResizeType::RESIZE_TYPE_UNDEFINED;
    const std::string bundleName = "test";

    DragResizeType globalDragResizeType = DragResizeType::RESIZE_WHEN_DRAG_END;
    DragResizeType appDragResizeType = DragResizeType::RESIZE_EACH_FRAME;
    WindowManager::GetInstance().SetGlobalDragResizeType(globalDragResizeType);
    WindowManager::GetInstance().SetAppDragResizeType(bundleName, appDragResizeType);
    WindowManager::GetInstance().GetAppDragResizeType(bundleName, dragResizeType);
    ASSERT_EQ(dragResizeType, globalDragResizeType);
    WindowManager::GetInstance().SetGlobalDragResizeType(DragResizeType::RESIZE_TYPE_UNDEFINED);
    WindowManager::GetInstance().GetAppDragResizeType(bundleName, dragResizeType);
    ASSERT_EQ(dragResizeType, appDragResizeType);
}

/**
 * @tc.name: SetAppKeyFramePolicy01
 * @tc.desc: check SetAppKeyFramePolicy enable
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, SetAppKeyFramePolicy01, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    const std::string bundleName = "test";
    KeyFramePolicy keyFramePolicy;
    keyFramePolicy.dragResizeType_ = DragResizeType::RESIZE_KEY_FRAME;
    keyFramePolicy.animationDelay_ = 200;
    auto ret = instance_->SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret);
}

/**
 * @tc.name: SetAppKeyFramePolicy02
 * @tc.desc: check SetAppKeyFramePolicy disable
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, SetAppKeyFramePolicy02, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    const std::string bundleName = "test";
    KeyFramePolicy keyFramePolicy;
    keyFramePolicy.dragResizeType_ = DragResizeType::RESIZE_TYPE_UNDEFINED;
    auto ret = instance_->SetAppKeyFramePolicy(bundleName, keyFramePolicy);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret);
}

/**
 * @tc.name: NotifyWMSConnected
 * @tc.desc: check NotifyWMSConnected
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyWMSConnected, TestSize.Level1)
{
    WMError ret = WindowManager::GetInstance().ShiftAppWindowFocus(0, 1);
    ASSERT_NE(WMError::WM_OK, ret);
    WindowManager::GetInstance().pImpl_->NotifyWMSConnected(1, 2);
}

/**
 * @tc.name: GetAllWindowLayoutInfo
 * @tc.desc: check GetAllWindowLayoutInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetAllWindowLayoutInfo, TestSize.Level1)
{
    DisplayId displayId = 1;
    std::vector<sptr<WindowLayoutInfo>> infos;
    auto ret = WindowManager::GetInstance().GetAllWindowLayoutInfo(displayId, infos);
    ASSERT_EQ(SingletonContainer::Get<WindowAdapter>().GetAllWindowLayoutInfo(displayId, infos), ret);
}

/**
 * @tc.name: GetTopNavDestinationName
 * @tc.desc: test GetTopNavDestinationName rpc is ok
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetTopNavDestinationName, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    int32_t windowId = 888;
    std::string topNavDestName;
    auto ret = instance_->GetTopNavDestinationName(windowId, topNavDestName);
    EXPECT_EQ(ret, WMError::WM_ERROR_SAMGR);
    EXPECT_EQ(topNavDestName, "");
}

/**
 * @tc.name: SetWatermarkImageForApp
 * @tc.desc: test SetWatermarkImageForApp rpc is ok
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, SetWatermarkImageForApp, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    instance_->SetWatermarkImageForApp(nullptr);
}

/**
 * @tc.name: ShiftAppWindowPointerEvent
 * @tc.desc: check ShiftAppWindowPointerEvent
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, ShiftAppWindowPointerEvent, TestSize.Level1)
{
    int32_t sourceWindowId = 1;
    int32_t targetWindowId = 1;
    int32_t fingerId = 0;
    ASSERT_NE(nullptr, instance_);
    instance_->ShiftAppWindowPointerEvent(sourceWindowId, targetWindowId, fingerId);
}

/**
 * @tc.name: OnWMSConnectionChanged
 * @tc.desc: check OnWMSConnectionChanged
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, OnWMSConnectionChanged, TestSize.Level1)
{
    int32_t userId = 1;
    int32_t screenId = 1;
    int32_t isConnected = 1;

    WMError ret_1 = WindowManager::GetInstance().ShiftAppWindowFocus(0, 1);
    ASSERT_NE(WMError::WM_OK, ret_1);
    WindowManager::GetInstance().OnWMSConnectionChanged(userId, screenId, isConnected);

    isConnected = 0;
    WMError ret_2 = WindowManager::GetInstance().ShiftAppWindowFocus(0, 1);
    ASSERT_NE(WMError::WM_OK, ret_2);
    WindowManager::GetInstance().OnWMSConnectionChanged(userId, screenId, isConnected);
}

/**
 * @tc.name: RequestFocus
 * @tc.desc: check RequestFocus
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RequestFocus, TestSize.Level1)
{
    int32_t persistentId = 1;
    bool isFocused = true;
    bool byForeground = true;
    WindowFocusChangeReason reason = WindowFocusChangeReason::CLICK;
    ASSERT_NE(nullptr, instance_);
    auto result = instance_->RequestFocus(persistentId, isFocused, byForeground, reason);
    ASSERT_NE(result, WMError::WM_OK);
}

/**
 * @tc.name: MinimizeByWindowId
 * @tc.desc: Check MinimizeByWindowId
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, MinimizeByWindowId, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    std::vector<int32_t> windowIds;
    WMError ret_1 = instance_->MinimizeByWindowId(windowIds);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret_1);
    windowIds = {-1, 0};
    WMError ret_2 = instance_->MinimizeByWindowId(windowIds);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret_2);
}

/**
 * @tc.name: ProcessRegisterWindowInfoChangeCallback
 * @tc.desc: Check ProcessRegisterWindowInfoChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, ProcessRegisterWindowInfoChangeCallback, Function | SmallTest | Level2)
{
    WMError ret;
    WindowInfoKey observedInfo;
    sptr<IWindowInfoChangedListener> listener = nullptr;

    WMError::WM_ERROR_NULLPTR;
    // branch 1: case VISIBILITY_STATE
    observedInfo = WindowInfoKey::VISIBILITY_STATE;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, instance_->ProcessRegisterWindowInfoChangeCallback(observedInfo, listener));

    // branch 2: case DISPLAY_ID
    observedInfo = WindowInfoKey::VISIBILITY_STATE;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, instance_->ProcessRegisterWindowInfoChangeCallback(observedInfo, listener));

    // branch 3: case WINDOW_RECT
    observedInfo = WindowInfoKey::WINDOW_RECT;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, instance_->ProcessRegisterWindowInfoChangeCallback(observedInfo, listener));

    // branch 4: case WINDOW_MODE
    observedInfo = WindowInfoKey::WINDOW_MODE;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, instance_->ProcessRegisterWindowInfoChangeCallback(observedInfo, listener));

    // branch 5: case FLOATING_SCALE
    observedInfo = WindowInfoKey::FLOATING_SCALE;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, instance_->ProcessRegisterWindowInfoChangeCallback(observedInfo, listener));

    // branch 6: case MID_SCENE
    observedInfo = WindowInfoKey::MID_SCENE;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, instance_->ProcessRegisterWindowInfoChangeCallback(observedInfo, listener));

    // branch 7: case WINDOW_GLOBAL_RECT
    observedInfo = WindowInfoKey::WINDOW_GLOBAL_RECT;
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, instance_->ProcessRegisterWindowInfoChangeCallback(observedInfo, listener));

    // branch 8: default
    observedInfo = static_cast<WindowInfoKey>(-1);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM,
              instance_->ProcessRegisterWindowInfoChangeCallback(observedInfo, listener));
}

/**
 * @tc.name: ProcessUnregisterWindowInfoChangeCallback01
 * @tc.desc: Check ProcessUnregisterWindowInfoChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, ProcessUnregisterWindowInfoChangeCallback01, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    sptr<IWindowInfoChangedListener> listener = sptr<MockWindowInfoChangedListener>::MakeSptr();
    WindowInfoKey observedInfo = WindowInfoKey::VISIBILITY_STATE;
    auto ret = instance_->ProcessUnregisterWindowInfoChangeCallback(observedInfo, listener);
    EXPECT_EQ(WMError::WM_OK, ret);
    observedInfo = WindowInfoKey::DISPLAY_ID;
    ret = WindowManager::GetInstance().ProcessUnregisterWindowInfoChangeCallback(observedInfo, listener);
    EXPECT_EQ(WMError::WM_OK, ret);
    observedInfo = WindowInfoKey::WINDOW_RECT;
    ret = WindowManager::GetInstance().ProcessUnregisterWindowInfoChangeCallback(observedInfo, listener);
    EXPECT_EQ(WMError::WM_OK, ret);
    ret = WindowManager::GetInstance().ProcessUnregisterWindowInfoChangeCallback(observedInfo, nullptr);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    observedInfo = WindowInfoKey::BUNDLE_NAME;
    ret = WindowManager::GetInstance().ProcessUnregisterWindowInfoChangeCallback(observedInfo, listener);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    observedInfo = WindowInfoKey::MID_SCENE;
    ret = WindowManager::GetInstance().ProcessUnregisterWindowInfoChangeCallback(observedInfo, nullptr);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    observedInfo = WindowInfoKey::WINDOW_GLOBAL_RECT;
    ret = WindowManager::GetInstance().ProcessUnregisterWindowInfoChangeCallback(observedInfo, listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: RegisterWindowInfoChangeCallback01
 * @tc.desc: Check RegisterWindowInfoChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterWindowInfoChangeCallback01, Function | SmallTest | Level2)
{
    sptr<IWindowInfoChangedListener> listener = sptr<MockWindowInfoChangedListener>::MakeSptr();
    auto interestInfoSizeOld = listener->GetInterestInfo().size();
    std::unordered_set<WindowInfoKey> observedInfo;
    observedInfo.insert(WindowInfoKey::VISIBILITY_STATE);
    auto ret = WindowManager::GetInstance().RegisterWindowInfoChangeCallback(observedInfo, listener);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PERMISSION, ret);
    EXPECT_EQ(interestInfoSizeOld + 1, listener->GetInterestInfo().size());
    std::unordered_set<WindowInfoKey> observedInfo1;
    observedInfo1.insert(WindowInfoKey::BUNDLE_NAME);
    ret = WindowManager::GetInstance().RegisterWindowInfoChangeCallback(observedInfo1, listener);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    ret = WindowManager::GetInstance().RegisterWindowInfoChangeCallback(observedInfo, nullptr);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    listener->SetInterestWindowIds({1, 2, 3});
    interestInfoSizeOld = listener->GetInterestInfo().size();
    ret = WindowManager::GetInstance().RegisterWindowInfoChangeCallback(observedInfo, listener);
    EXPECT_EQ(interestInfoSizeOld + 1, listener->GetInterestInfo().size());
}

/**
 * @tc.name: UnregisterWindowInfoChangeCallback01
 * @tc.desc: Check UnregisterWindowInfoChangeCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterWindowInfoChangeCallback01, Function | SmallTest | Level2)
{
    sptr<IWindowInfoChangedListener> listener = sptr<MockWindowInfoChangedListener>::MakeSptr();
    auto interestInfoSizeOld = listener->GetInterestInfo().size();
    std::unordered_set<WindowInfoKey> observedInfo;
    observedInfo.insert(WindowInfoKey::VISIBILITY_STATE);
    auto ret = WindowManager::GetInstance().UnregisterWindowInfoChangeCallback(observedInfo, listener);
    EXPECT_EQ(WMError::WM_OK, ret);
    EXPECT_EQ(interestInfoSizeOld + 1, listener->GetInterestInfo().size());
    std::unordered_set<WindowInfoKey> observedInfo1;
    observedInfo1.insert(WindowInfoKey::BUNDLE_NAME);
    ret = WindowManager::GetInstance().UnregisterWindowInfoChangeCallback(observedInfo1, listener);
    EXPECT_EQ(WMError::WM_ERROR_INVALID_PARAM, ret);
    ret = WindowManager::GetInstance().UnregisterWindowInfoChangeCallback(observedInfo, nullptr);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);
    listener->SetInterestWindowIds({1, 2, 3});
    interestInfoSizeOld = listener->GetInterestInfo().size();
    ret = WindowManager::GetInstance().UnregisterWindowInfoChangeCallback(observedInfo, listener);
    EXPECT_EQ(interestInfoSizeOld + 1, listener->GetInterestInfo().size());
}

/**
 * @tc.name: RegisterVisibilityStateChangedListener
 * @tc.desc: check RegisterVisibilityStateChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterVisibilityStateChangedListener, Function | SmallTest | Level2)
{
    WMError ret;
    sptr<IWindowInfoChangedListener> listener = nullptr;

    // branch 1
    ret = instance_->RegisterVisibilityStateChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: enter ActiveFaultAgentReregister and return ok
    listener = sptr<MockWindowInfoChangedListener>::MakeSptr();
    windowAdapter->isProxyValid_ = true; // skip iniSSMProxy
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->RegisterVisibilityStateChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 3: use mock and return ok
    ret = mockInstance_->RegisterVisibilityStateChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 4: branch override and return ok.
    ret = mockInstance_->RegisterVisibilityStateChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterVisibilityStateChangedListener
 * @tc.desc: check UnregisterVisibilityStateChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterVisibilityStateChangedListener, Function | SmallTest | Level2)
{
    WMError ret;
    sptr<IWindowInfoChangedListener> listener = nullptr;

    // branch 1
    ret = instance_->UnregisterVisibilityStateChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: windowAdapter UnregisterWindowManagerAgent return failed
    listener = sptr<MockWindowInfoChangedListener>::MakeSptr();
    instance_->pImpl_->windowVisibilityStateListeners_.clear();
    instance_->pImpl_->windowVisibilityStateListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->UnregisterVisibilityStateChangedListener(listener);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 3: mock windowAdapter and return return ok
    mockInstance_->pImpl_->windowVisibilityStateListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    ret = mockInstance_->UnregisterVisibilityStateChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: RegisterDisplayIdChangedListener
 * @tc.desc: check RegisterDisplayIdChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterDisplayIdChangedListener, Function | SmallTest | Level2)
{
    WMError ret;
    sptr<IWindowInfoChangedListener> listener = nullptr;

    // branch 1
    ret = instance_->RegisterDisplayIdChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: use ActiveFaultAgentReregister and return ok.
    listener = sptr<MockWindowInfoChangedListener>::MakeSptr();
    windowAdapter->isProxyValid_ = true; // skip InitSSMProxy
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->RegisterDisplayIdChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 3: use mock and return ok
    ret = mockInstance_->RegisterDisplayIdChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterDisplayIdChangedListener01
 * @tc.desc: check UnregisterDisplayIdChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterDisplayIdChangedListener01, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->WindowDisplayIdChangeListenerAgent_;
    auto oldListeners = instance_->pImpl_->windowDisplayIdChangeListeners_;
    instance_->pImpl_->WindowDisplayIdChangeListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    instance_->pImpl_->windowDisplayIdChangeListeners_.clear();

    // check nullpter
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterDisplayIdChangedListener(nullptr));

    sptr<TestWindowDisplayIdChangeListener> listener1 = sptr<TestWindowDisplayIdChangeListener>::MakeSptr();
    sptr<TestWindowDisplayIdChangeListener> listener2 = sptr<TestWindowDisplayIdChangeListener>::MakeSptr();
    instance_->UnregisterDisplayIdChangedListener(listener1);

    instance_->RegisterVisibilityStateChangedListener(listener1);
    instance_->RegisterVisibilityStateChangedListener(listener2);

    EXPECT_EQ(WMError::WM_OK, instance_->UnregisterDisplayIdChangedListener(listener1));
    EXPECT_EQ(WMError::WM_OK, instance_->UnregisterDisplayIdChangedListener(listener2));
    EXPECT_EQ(0, instance_->pImpl_->windowDisplayIdChangeListeners_.size());

    instance_->pImpl_->windowDisplayIdChangeListeners_.emplace_back(listener1);
    EXPECT_EQ(WMError::WM_OK, instance_->UnregisterDisplayIdChangedListener(listener1));
    EXPECT_EQ(0, instance_->pImpl_->windowDisplayIdChangeListeners_.size());

    instance_->pImpl_->WindowDisplayIdChangeListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->windowDisplayIdChangeListeners_ = oldListeners;
}

/**
 * @tc.name: RegisterWindowSystemBarPropertyChangedListener
 * @tc.desc: check RegisterWindowSystemBarPropertyChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterWindowSystemBarPropertyChangedListener, Function | SmallTest | Level2)
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
HWTEST_F(WindowManagerTest, UnregisterWindowSystemBarPropertyChangedListener, Function | SmallTest | Level2)
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
HWTEST_F(WindowManagerTest, NotifyWindowSystemBarPropertyChange, TestSize.Level1)
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
 * @tc.name: RegisterFloatingScaleChangedListener
 * @tc.desc: check RegisterFloatingScaleChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterFloatingScaleChangedListener, Function | SmallTest | Level2)
{
    WMError ret;
    sptr<IWindowInfoChangedListener> listener = nullptr;

    // branch 1
    ret = instance_->RegisterFloatingScaleChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: enter ActiveFaultAgentReregister and return ok
    listener = sptr<MockWindowInfoChangedListener>::MakeSptr();
    windowAdapter->isProxyValid_ = true; // skip iniSSMProxy
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->RegisterFloatingScaleChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 3: use mock and return ok.
    ret = mockInstance_->RegisterFloatingScaleChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterFloatingScaleChangedListener
 * @tc.desc: check UnregisterFloatingScaleChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterFloatingScaleChangedListener, Function | SmallTest | Level2)
{
    WMError ret;
    sptr<IWindowInfoChangedListener> listener = nullptr;

    // branch 1
    ret = instance_->UnregisterFloatingScaleChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: return failed
    listener = sptr<MockWindowInfoChangedListener>::MakeSptr();
    instance_->pImpl_->floatingScaleChangeListeners_.clear();
    instance_->pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->UnregisterFloatingScaleChangedListener(listener);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 3: use mock and return ok
    mockInstance_->pImpl_->floatingScaleChangeListeners_.clear();
    mockInstance_->pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    ret = mockInstance_->UnregisterFloatingScaleChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: NotifyFloatingScaleChange
 * @tc.desc: check NotifyFloatingScaleChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyFloatingScaleChange, TestSize.Level1)
{
    WindowInfoList windowInfoList;
    windowInfoList.push_back({{WindowInfoKey::DISPLAY_ID, static_cast<uint64_t>(5)}});
    windowInfoList.push_back({{WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(1)}});
    windowInfoList.push_back({{WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(2)}});
    windowInfoList.push_back({{WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(3)}});

    auto& windowManager = WindowManager::GetInstance();
    windowManager.pImpl_->floatingScaleChangeListeners_.clear();
    windowManager.pImpl_->NotifyFloatingScaleChange(windowInfoList);

    windowManager.pImpl_->floatingScaleChangeListeners_.push_back(nullptr);
    windowManager.pImpl_->NotifyFloatingScaleChange(windowInfoList);

    auto listener = sptr<MockWindowInfoChangedListener>::MakeSptr();

    windowManager.pImpl_->NotifyFloatingScaleChange(windowInfoList);
    EXPECT_EQ(listener->count_, 0);

    windowManager.pImpl_->floatingScaleChangeListeners_.push_back(listener);
    windowManager.pImpl_->NotifyFloatingScaleChange(windowInfoList);
    EXPECT_EQ(listener->count_, 1);

    listener->SetInterestWindowIds({ 9 });
    windowManager.pImpl_->NotifyFloatingScaleChange(windowInfoList);
    EXPECT_EQ(listener->count_, 1);
}

/**
 * @tc.name: RegisterMidSceneChangedListener
 * @tc.desc: check RegisterMidSceneChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterMidSceneChangedListener, Function | SmallTest | Level2)
{
    WMError ret;
    sptr<IWindowInfoChangedListener> listener = nullptr;

    // branch 1
    ret = instance_->RegisterMidSceneChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: enter ActiveFaultAgentReregister and return ok
    listener = sptr<MockWindowInfoChangedListener>::MakeSptr();
    windowAdapter->isProxyValid_ = true; // skip iniSSMProxy
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->RegisterMidSceneChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 3: use mock and return ok.
    ret = mockInstance_->RegisterMidSceneChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterMidSceneChangedListener
 * @tc.desc: check UnregisterMidSceneChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterMidSceneChangedListener, Function | SmallTest | Level2)
{
    WMError ret;
    sptr<IWindowInfoChangedListener> listener = nullptr;

    // branch 1
    ret = instance_->UnregisterMidSceneChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: return failed
    listener = sptr<MockWindowInfoChangedListener>::MakeSptr();
    instance_->pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->UnregisterMidSceneChangedListener(listener);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 3: use mock and return ok
    listener->AddInterestInfo(Rosen::WindowInfoKey::MID_SCENE);
    listener->AddInterestInfo(Rosen::WindowInfoKey::NONE);
    mockInstance_->pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    ret = mockInstance_->UnregisterMidSceneChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: NotifyMidSceneStatusChange01
 * @tc.desc: check NotifyMidSceneStatusChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyMidSceneStatusChange01, TestSize.Level1)
{
    WindowInfoList windowInfoList;
    windowInfoList.push_back({{WindowInfoKey::MID_SCENE, true}});
    windowInfoList.push_back({{WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(1)}});
    windowInfoList.push_back({{WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(2)}});
    windowInfoList.push_back({{WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(3)}});

    auto& windowManager = WindowManager::GetInstance();
    windowManager.pImpl_->midSceneStatusChangeListeners_.clear();
    windowManager.pImpl_->NotifyMidSceneStatusChange(windowInfoList);

    windowManager.pImpl_->midSceneStatusChangeListeners_.push_back(nullptr);
    windowManager.pImpl_->NotifyMidSceneStatusChange(windowInfoList);

    auto listener = sptr<MockWindowInfoChangedListener>::MakeSptr();

    windowManager.pImpl_->NotifyMidSceneStatusChange(windowInfoList);
    EXPECT_EQ(listener->count_, 0);

    windowManager.pImpl_->midSceneStatusChangeListeners_.push_back(listener);
    windowManager.pImpl_->NotifyMidSceneStatusChange(windowInfoList);
    EXPECT_EQ(listener->count_, 1);

    listener->SetInterestWindowIds({ 9 });
    windowManager.pImpl_->NotifyMidSceneStatusChange(windowInfoList);
    EXPECT_EQ(listener->count_, 1);
}

/**
 * @tc.name: RegisterWindowModeChangedListenerForPropertyChange
 * @tc.desc: check RegisterWindowModeChangedListenerForPropertyChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterWindowModeChangedListenerForPropertyChange, Function | SmallTest | Level2)
{
    WMError ret;
    sptr<IWindowInfoChangedListener> listener = nullptr;

    // branch 1
    ret = instance_->RegisterWindowModeChangedListenerForPropertyChange(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: enter ActiveFaultAgentReregister and return ok
    listener = sptr<MockWindowInfoChangedListener>::MakeSptr();
    windowAdapter->isProxyValid_ = true; // skip iniSSMProxy
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->RegisterWindowModeChangedListenerForPropertyChange(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 3: use mock and return ok.
    ret = mockInstance_->RegisterWindowModeChangedListenerForPropertyChange(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterWindowModeChangedListenerForPropertyChange
 * @tc.desc: check UnregisterWindowModeChangedListenerForPropertyChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterWindowModeChangedListenerForPropertyChange, Function | SmallTest | Level2)
{
    WMError ret;
    sptr<IWindowInfoChangedListener> listener = nullptr;

    // branch 1
    ret = instance_->UnregisterFloatingScaleChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: return failed
    listener = sptr<MockWindowInfoChangedListener>::MakeSptr();
    instance_->pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->UnregisterWindowModeChangedListenerForPropertyChange(listener);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 3: use mock and return ok
    mockInstance_->pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    ret = mockInstance_->UnregisterFloatingScaleChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: NotifyWindowModeChangeForPropertyChange
 * @tc.desc: check NotifyWindowModeChangeForPropertyChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyWindowModeChangeForPropertyChange, TestSize.Level1)
{
    WindowInfoList windowInfoList;
    windowInfoList.push_back({{WindowInfoKey::DISPLAY_ID, static_cast<uint64_t>(5)}});
    windowInfoList.push_back({{WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(1)}});
    windowInfoList.push_back({{WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(2)}});
    windowInfoList.push_back({{WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(3)}});

    auto& windowManager = WindowManager::GetInstance();
    windowManager.pImpl_->windowModeChangeListeners_.clear();
    windowManager.pImpl_->NotifyWindowModeChangeForPropertyChange(windowInfoList);
 
    windowManager.pImpl_->windowModeChangeListeners_.push_back(nullptr);
    windowManager.pImpl_->NotifyWindowModeChangeForPropertyChange(windowInfoList);

    auto listener = sptr<MockWindowInfoChangedListener>::MakeSptr();

    windowManager.pImpl_->NotifyWindowModeChangeForPropertyChange(windowInfoList);
    EXPECT_EQ(listener->count_, 0);

    windowManager.pImpl_->windowModeChangeListeners_.push_back(listener);
    windowManager.pImpl_->NotifyWindowModeChangeForPropertyChange(windowInfoList);
    EXPECT_EQ(listener->count_, 1);

    listener->SetInterestWindowIds({ 9 });
    windowManager.pImpl_->NotifyWindowModeChangeForPropertyChange(windowInfoList);
    EXPECT_EQ(listener->count_, 1);
}

/**
 * @tc.name: NotifyDisplayIdChange
 * @tc.desc: check NotifyDisplayIdChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyDisplayIdChange, TestSize.Level1)
{
    WindowInfoList windowInfoList;
    windowInfoList.push_back({{WindowInfoKey::DISPLAY_ID, static_cast<uint64_t>(5)}});
    windowInfoList.push_back({{WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(1)}});
    windowInfoList.push_back({{WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(2)}});
    windowInfoList.push_back({{WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(3)}});

    auto& windowManager = WindowManager::GetInstance();
    windowManager.pImpl_->windowDisplayIdChangeListeners_.clear();
    windowManager.pImpl_->NotifyDisplayIdChange(windowInfoList);
 
    windowManager.pImpl_->windowDisplayIdChangeListeners_.push_back(nullptr);
    windowManager.pImpl_->NotifyDisplayIdChange(windowInfoList);

    auto listener = sptr<MockWindowInfoChangedListener>::MakeSptr();

    windowManager.pImpl_->NotifyDisplayIdChange(windowInfoList);
    EXPECT_EQ(listener->count_, 0);

    windowManager.pImpl_->windowDisplayIdChangeListeners_.push_back(listener);
    windowManager.pImpl_->NotifyDisplayIdChange(windowInfoList);
    EXPECT_EQ(listener->count_, 1);

    listener->SetInterestWindowIds({ 9 });
    windowManager.pImpl_->NotifyDisplayIdChange(windowInfoList);
    EXPECT_EQ(listener->count_, 1);
}

/**
 * @tc.name: RegisterRectChangedListener
 * @tc.desc: check RegisterRectChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterRectChangedListener, Function | SmallTest | Level2)
{
    WMError ret;
    sptr<IWindowInfoChangedListener> listener = nullptr;

    // branch 1
    ret = instance_->RegisterRectChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: return ok
    listener = sptr<MockWindowInfoChangedListener>::MakeSptr();
    windowAdapter->isProxyValid_ = true; // skip iniSSMProxy
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->RegisterRectChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 3: use mock and return ok.
    ret = mockInstance_->RegisterRectChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterRectChangedListener
 * @tc.desc: check UnregisterRectChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterRectChangedListener, Function | SmallTest | Level2)
{
    WMError ret;
    sptr<IWindowInfoChangedListener> listener = nullptr;

    // branch 1
    ret = instance_->UnregisterRectChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: return failed
    listener = sptr<MockWindowInfoChangedListener>::MakeSptr();
    instance_->pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->UnregisterRectChangedListener(listener);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 3: use mock and return ok
    mockInstance_->pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    ret = mockInstance_->UnregisterRectChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: RegisterGlobalRectChangedListener
 * @tc.desc: check RegisterGlobalRectChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterGlobalRectChangedListener, Function | SmallTest | Level2)
{
    WMError ret;
    sptr<IWindowInfoChangedListener> listener = nullptr;

    // branch 1
    ret = instance_->RegisterGlobalRectChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: enter ActiveFaultAgentReregister and return ok
    listener = sptr<MockWindowInfoChangedListener>::MakeSptr();
    windowAdapter->isProxyValid_ = true; // skip iniSSMProxy
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->RegisterGlobalRectChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    // branch 3: use mock and return ok.
    ret = mockInstance_->RegisterGlobalRectChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterGlobalRectChangedListener
 * @tc.desc: check UnregisterGlobalRectChangedListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterGlobalRectChangedListener, Function | SmallTest | Level2)
{
    WMError ret;
    sptr<IWindowInfoChangedListener> listener = nullptr;

    // branch 1
    ret = instance_->UnregisterGlobalRectChangedListener(listener);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    // branch 2: return failed
    listener = sptr<MockWindowInfoChangedListener>::MakeSptr();
    instance_->pImpl_->windowGlobalRectChangeListeners_.clear();
    instance_->pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->UnregisterGlobalRectChangedListener(listener);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 3: use mock and return ok
    mockInstance_->pImpl_->windowGlobalRectChangeListeners_.clear();
    mockInstance_->pImpl_->windowPropertyChangeAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    ret = mockInstance_->UnregisterFloatingScaleChangedListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: AnimateTo01
 * @tc.desc: check AnimateTo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, AnimateTo01, Function | SmallTest | Level2)
{
    int32_t windowId = 1;
    WindowAnimationProperty animationProperty;
    WindowAnimationOption animationOption;
    animationProperty.targetScale = 1.5f;

    ASSERT_NE(nullptr, instance_);
    instance_->AnimateTo(windowId, animationProperty, animationOption);
}

/**
 * @tc.name: RegisterWindowLifeCycleListener01
 * @tc.desc: check RegisterWindowLifeCycleCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterWindowLifeCycleListener01, TestSize.Level1)
{
    WMError ret;
    sptr<TestIWindowLifeCycleListener> listener = sptr<TestIWindowLifeCycleListener>::MakeSptr();
    ret = WindowManager::GetInstance().RegisterWindowLifeCycleCallback(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = WindowManager::GetInstance().RegisterWindowLifeCycleCallback(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    ret = WindowManager::GetInstance().RegisterWindowLifeCycleCallback(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: UnregisterWindowLifeCycleListener01
 * @tc.desc: check UnregisterWindowLifeCycleCallback
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterWindowLifeCycleListener01, TestSize.Level1)
{
    WMError ret;
    sptr<TestIWindowLifeCycleListener> listener = sptr<TestIWindowLifeCycleListener>::MakeSptr();
    ret = WindowManager::GetInstance().UnregisterWindowLifeCycleCallback(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    ret = WindowManager::GetInstance().UnregisterWindowLifeCycleCallback(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    ret = WindowManager::GetInstance().UnregisterWindowLifeCycleCallback(listener);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: NotifyWMSWindowDestroyed01
 * @tc.desc: check NotifyWMSWindowDestroyed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyWMSWindowDestroyed01, TestSize.Level1)
{
    WMError ret;
    sptr<TestIWindowLifeCycleListener> listener = sptr<TestIWindowLifeCycleListener>::MakeSptr();
    ret = WindowManager::GetInstance().RegisterWindowLifeCycleCallback(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    WindowLifeCycleInfo lifeCycleInfo;
    lifeCycleInfo.windowId = 101;
    lifeCycleInfo.windowType = WindowType::APP_WINDOW_BASE;
    lifeCycleInfo.windowName = "window101";
    WindowManager::GetInstance().NotifyWMSWindowDestroyed(lifeCycleInfo);
    EXPECT_EQ(lifeCycleInfo.windowId, listener->listenerLifeCycleInfo.windowId);
    EXPECT_EQ(lifeCycleInfo.windowType, listener->listenerLifeCycleInfo.windowType);
    EXPECT_EQ(lifeCycleInfo.windowName, listener->listenerLifeCycleInfo.windowName);

    lifeCycleInfo.windowId = 102;
    lifeCycleInfo.windowType = WindowType::APP_SUB_WINDOW_BASE;
    lifeCycleInfo.windowName = "window102";
    WindowManager::GetInstance().NotifyWMSWindowDestroyed(lifeCycleInfo);
    EXPECT_EQ(lifeCycleInfo.windowId, listener->listenerLifeCycleInfo.windowId);
    EXPECT_EQ(lifeCycleInfo.windowType, listener->listenerLifeCycleInfo.windowType);
    EXPECT_EQ(lifeCycleInfo.windowName, listener->listenerLifeCycleInfo.windowName);

    lifeCycleInfo.windowId = 103;
    lifeCycleInfo.windowType = WindowType::SYSTEM_WINDOW_BASE;
    lifeCycleInfo.windowName = "window103";
    WindowManager::GetInstance().NotifyWMSWindowDestroyed(lifeCycleInfo);
    EXPECT_EQ(lifeCycleInfo.windowId, listener->listenerLifeCycleInfo.windowId);
    EXPECT_EQ(lifeCycleInfo.windowType, listener->listenerLifeCycleInfo.windowType);
    EXPECT_EQ(lifeCycleInfo.windowName, listener->listenerLifeCycleInfo.windowName);

    lifeCycleInfo.windowId = 104;
    lifeCycleInfo.windowType = WindowType::ABOVE_APP_SYSTEM_WINDOW_BASE;
    lifeCycleInfo.windowName = "window104";
    WindowManager::GetInstance().NotifyWMSWindowDestroyed(lifeCycleInfo);
    EXPECT_EQ(lifeCycleInfo.windowId, listener->listenerLifeCycleInfo.windowId);
    EXPECT_EQ(lifeCycleInfo.windowType, listener->listenerLifeCycleInfo.windowType);
    EXPECT_EQ(lifeCycleInfo.windowName, listener->listenerLifeCycleInfo.windowName);

    lifeCycleInfo.windowId = 105;
    lifeCycleInfo.windowType = WindowType::SYSTEM_SUB_WINDOW_BASE;
    lifeCycleInfo.windowName = "window105";
    WindowManager::GetInstance().NotifyWMSWindowDestroyed(lifeCycleInfo);
    EXPECT_EQ(lifeCycleInfo.windowId, listener->listenerLifeCycleInfo.windowId);
    EXPECT_EQ(lifeCycleInfo.windowType, listener->listenerLifeCycleInfo.windowType);
    EXPECT_EQ(lifeCycleInfo.windowName, listener->listenerLifeCycleInfo.windowName);
}

/**
 * @tc.name: NotifyWMSWindowDestroyed02
 * @tc.desc: check NotifyWMSWindowDestroyed
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifyWMSWindowDestroyed02, TestSize.Level1)
{
    WMError ret;
    sptr<TestIWindowLifeCycleListener> listener = sptr<TestIWindowLifeCycleListener>::MakeSptr();
    ret = WindowManager::GetInstance().UnregisterWindowLifeCycleCallback(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    WindowLifeCycleInfo lifeCycleInfo;
    lifeCycleInfo.windowId = 101;
    lifeCycleInfo.windowType = WindowType::APP_WINDOW_BASE;
    lifeCycleInfo.windowName = "window101";
    auto func = [](const std::string& windowName) {
        std::cout << "window name: " << windowName << std::endl;
        return napi_value();
    };
    WindowManager::GetInstance().RegisterGetJSWindowCallback(func);
    WindowManager::GetInstance().NotifyWMSWindowDestroyed(lifeCycleInfo);
    EXPECT_NE(lifeCycleInfo.windowId, listener->listenerLifeCycleInfo.windowId);
    EXPECT_NE(lifeCycleInfo.windowType, listener->listenerLifeCycleInfo.windowType);
    EXPECT_NE(lifeCycleInfo.windowName, listener->listenerLifeCycleInfo.windowName);
}

/**
 * @tc.name: AddSessionBlackList01
 * @tc.desc: check AddSessionBlackList
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, AddSessionBlackList01, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    WMError ret;
    std::unordered_set<std::string> bundleNames;
    std::unordered_set<std::string> privacyWindowTags;
    ret = instance_->AddSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(WMError::WM_ERROR_SAMGR, ret);
}

/**
 * @tc.name: RemoveSessionBlackList01
 * @tc.desc: check RemoveSessionBlackList
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RemoveSessionBlackList01, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    WMError ret;
    std::unordered_set<std::string> bundleNames;
    std::unordered_set<std::string> privacyWindowTags;
    ret = instance_->RemoveSessionBlackList(bundleNames, privacyWindowTags);
    EXPECT_EQ(WMError::WM_ERROR_SAMGR, ret);
}

/**
 * @tc.name: GetInstance
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetInstanceMulti, TestSize.Level1)
{
    // branch overried
    sptr<WindowManager> instance = &WindowManager::GetInstance(userId_);
    ASSERT_NE(instance, nullptr);
}


/**
 * @tc.name: UpdateOutline
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UpdateOutline, TestSize.Level1)
{
    OutlineParams params;
    WMError ret = WindowManager::GetInstance().UpdateOutline(nullptr, params);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);

    sptr<IRemoteObject> remoteObject = sptr<IRemoteObjectMocker>::MakeSptr();
    ret = WindowManager::GetInstance().UpdateOutline(remoteObject, params);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);

    params.type_ = OutlineType::OUTLINE_FOR_WINDOW;
    params.outlineStyleParams_.outlineWidth_ = 0;
    ret = WindowManager::GetInstance().UpdateOutline(remoteObject, params);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);

    params.outlineStyleParams_.outlineWidth_ = OUTLINE_WIDTH_MAX + 1;
    ret = WindowManager::GetInstance().UpdateOutline(remoteObject, params);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);

    params.outlineStyleParams_.outlineWidth_ = OUTLINE_WIDTH_DEFAULT;
    params.outlineStyleParams_.outlineShape_ = OutlineShape::OUTLINE_SHAPE_END;
    ret = WindowManager::GetInstance().UpdateOutline(remoteObject, params);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);

    params.outlineStyleParams_.outlineShape_ = OutlineShape::OUTLINE_SHAPE_FOUR_CORNERS;
    params.outlineStyleParams_.outlineColor_ = 0x00ffffff  + 1; // 0x00ffffff: color has no alpha byte.
    ret = WindowManager::GetInstance().UpdateOutline(remoteObject, params);
    EXPECT_EQ(ret, WMError::WM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name: RegisterWMSConnectionChangedListener
 * @tc.desc: normal function
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterWMSConnectionChangedListener, TestSize.Level1)
{
    ASSERT_NE(nullptr, instance_);
    sptr<TestWMSListener> listener = new TestWMSListener();
    instance_->RegisterWMSConnectionChangedListener(listener);
}

/**
 * @tc.name: GetAllMainWindowInfo
 * @tc.desc: check GetAllMainWindowInfo
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetAllMainWindowInfo, TestSize.Level1)
{
    ASSERT_NE(instance_, nullptr);
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    std::vector<sptr<MainWindowInfo>> infos;
    WMError ret = instance_->GetAllMainWindowInfo(infos);
    ASSERT_EQ(WMError::WM_ERROR_SAMGR, ret);
}

/**
 * @tc.name: GetMainWindowSnapshot
 * @tc.desc: check GetMainWindowSnapshot
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetMainWindowSnapshot, TestSize.Level1)
{
    std::vector<int32_t> windowIds;
    windowIds.emplace_back(1);
    windowIds.emplace_back(9);
    WindowSnapshotConfiguration configs;
    configs.useCache = true;
    ASSERT_NE(nullptr, windowAdapter);
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    sptr<MockIRemoteObject> iRemoteObjMocker = sptr<MockIRemoteObject>::MakeSptr();
    WMError ret = instance_->GetMainWindowSnapshot(windowIds, configs, iRemoteObjMocker);
    EXPECT_EQ(WMError::WM_ERROR_SAMGR, ret);

    iRemoteObjMocker->SetRequestResult(ERR_NONE);
    ret = instance_->GetMainWindowSnapshot(windowIds, configs, iRemoteObjMocker);
    EXPECT_EQ(WMError::WM_ERROR_SAMGR, ret);
}

/**
 * @tc.name: SetSpecificSystemWindowZIndex
 * @tc.desc: SetSpecificSystemWindowZIndex
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, SetSpecificSystemWindowZIndex, TestSize.Level1)
{
    WMError ret;

    // branch 1: return failed
    windowAdapter->isProxyValid_ = true;
    windowAdapter->windowManagerServiceProxy_ = nullptr;
    ret = instance_->SetSpecificSystemWindowZIndex(WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD, 20);
    EXPECT_NE(WMError::WM_OK, ret);

    // branch 2: use mock and return ok
    ret = mockInstance_->SetSpecificSystemWindowZIndex(WindowType::WINDOW_TYPE_WALLET_SWIPE_CARD, 20);
    EXPECT_EQ(WMError::WM_OK, ret);
}

/**
 * @tc.name: RegisterWindowSupportRotationListener
 * @tc.desc: RegisterWindowSupportRotationListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, RegisterWindowSupportRotationListener, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->windowSupportRotationListenerAgent_;
    auto oldListeners = instance_->pImpl_->windowSupportRotationListeners_;
    // register nullptr test
    auto ret = WMError::WM_OK;
    ret = instance_->RegisterWindowSupportRotationListener(nullptr);
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, ret);

    sptr<TestIWindowSupportRotationListener> listener = sptr<TestIWindowSupportRotationListener>::MakeSptr();
    instance_->RegisterWindowSupportRotationListener(listener);
    EXPECT_EQ(1, instance_->pImpl_->windowSupportRotationListeners_.size());

    // to check that the same listner can not be registered twice
    instance_->RegisterWindowSupportRotationListener(listener);
    EXPECT_EQ(1, instance_->pImpl_->windowSupportRotationListeners_.size());

    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;
    instance_->UnregisterWindowSupportRotationListener(listener);
    instance_->pImpl_->windowSupportRotationListenerAgent_ = nullptr;
    instance_->pImpl_->windowSupportRotationListeners_.clear();
    ret = WindowManager::GetInstance().RegisterWindowSupportRotationListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    instance_->pImpl_->windowSupportRotationListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->windowSupportRotationListeners_ = oldListeners;
}

/**
 * @tc.name: UnregisterWindowSupportRotationListener
 * @tc.desc: UnregisterWindowSupportRotationListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, UnregisterWindowSupportRotationListener, Function | SmallTest | Level2)
{
    ASSERT_NE(nullptr, instance_);
    auto oldWindowManagerAgent = instance_->pImpl_->windowSupportRotationListenerAgent_;
    auto oldListeners = instance_->pImpl_->windowSupportRotationListeners_;
    instance_->pImpl_->windowSupportRotationListenerAgent_ = sptr<WindowManagerAgent>::MakeSptr();
    instance_->pImpl_->windowSupportRotationListeners_.clear();

    // check nullpter
    EXPECT_EQ(WMError::WM_ERROR_NULLPTR, instance_->UnregisterWindowSupportRotationListener(nullptr));

    sptr<TestIWindowSupportRotationListener> listener1 = sptr<TestIWindowSupportRotationListener>::MakeSptr();
    sptr<TestIWindowSupportRotationListener> listener2 = sptr<TestIWindowSupportRotationListener>::MakeSptr();
    EXPECT_EQ(WMError::WM_OK, instance_->UnregisterWindowSupportRotationListener(listener1));

    WindowAdapter windowAdapter;
    windowAdapter.isProxyValid_ = true;
    windowAdapter.windowManagerServiceProxy_ = nullptr;

    WindowManagerAgentType type = WindowManagerAgentType::WINDOW_MANAGER_AGENT_SUPPORT_ROTATION;
    instance_->pImpl_->windowSupportRotationListeners_.clear();
    EXPECT_EQ(0, instance_->pImpl_->windowSupportRotationListeners_.size());
    auto ret = windowAdapter.RegisterWindowManagerAgent(type, nullptr);
    EXPECT_EQ(WMError::WM_ERROR_SAMGR, ret);

    instance_->RegisterWindowSupportRotationListener(listener1);
    instance_->RegisterWindowSupportRotationListener(listener2);
    EXPECT_EQ(2, instance_->pImpl_->windowSupportRotationListeners_.size());
    EXPECT_EQ(WMError::WM_OK, instance_->UnregisterWindowSupportRotationListener(listener1));

    instance_->UnregisterWindowSupportRotationListener(listener2);
    EXPECT_EQ(0, instance_->pImpl_->windowSupportRotationListeners_.size());
    EXPECT_EQ(nullptr, instance_->pImpl_->windowSupportRotationListenerAgent_);

    instance_->pImpl_->windowSupportRotationListeners_.emplace_back(listener1);
    EXPECT_EQ(WMError::WM_OK, instance_->UnregisterWindowSupportRotationListener(listener1));
    EXPECT_EQ(0, instance_->pImpl_->windowSupportRotationListeners_.size());

    instance_->pImpl_->windowSupportRotationListenerAgent_ = oldWindowManagerAgent;
    instance_->pImpl_->windowSupportRotationListeners_ = oldListeners;
}

/**
 * @tc.name: GetWindowInfoListByInterestWindowIds_NullListener
 * @tc.desc: GetWindowInfoListByInterestWindowIds_NullListener
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetWindowInfoListByInterestWindowIds_NullListener, Function | SmallTest | Level2)
{
    WindowInfoList windowInfoList;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> info;
    info.emplace(WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(1));
    windowInfoList.emplace_back(info);

    auto result = mockInstance_->pImpl_->GetWindowInfoListByInterestWindowIds(nullptr, windowInfoList);
    EXPECT_EQ(windowInfoList, result);
}

/**
 * @tc.name: GetWindowInfoListByInterestWindowIds_EmptyInterestIds
 * @tc.desc: GetWindowInfoListByInterestWindowIds_EmptyInterestIds
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetWindowInfoListByInterestWindowIds_EmptyInterestIds, Function | SmallTest | Level2)
{
    auto listener = sptr<TestInterestWindowIdsListener>::MakeSptr();
    WindowInfoList windowInfoList;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> info;
    info.emplace(WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(1));
    windowInfoList.emplace_back(info);

    auto result = mockInstance_->pImpl_->GetWindowInfoListByInterestWindowIds(listener, windowInfoList);
    EXPECT_EQ(windowInfoList, result);
}

/**
 * @tc.name: GetWindowInfoListByInterestWindowIds_FilterMatch
 * @tc.desc: GetWindowInfoListByInterestWindowIds_FilterMatch
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, GetWindowInfoListByInterestWindowIds_FilterMatch, Function | SmallTest | Level2)
{
    auto listener = sptr<TestInterestWindowIdsListener>::MakeSptr();
    listener->AddInterestWindowId(1);

    WindowInfoList windowInfoList;
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> info1;
    info1.emplace(WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(1));
    std::unordered_map<WindowInfoKey, WindowChangeInfoType> info2;
    info2.emplace(WindowInfoKey::WINDOW_ID, static_cast<uint32_t>(2));
    windowInfoList.emplace_back(info1);
    windowInfoList.emplace_back(info2);

    auto result = mockInstance_->pImpl_->GetWindowInfoListByInterestWindowIds(listener, windowInfoList);
    ASSERT_EQ(1u, result.size());
    EXPECT_EQ(info1, result.front());
}

/**
 * @tc.name: NotifySupportRotationChange
 * @tc.desc: NotifySupportRotationChange
 * @tc.type: FUNC
 */
HWTEST_F(WindowManagerTest, NotifySupportRotationChange, Function | SmallTest | Level2)
{
    WMError ret;
    sptr<TestIWindowSupportRotationListener> listener = sptr<TestIWindowSupportRotationListener>::MakeSptr();
    ret = WindowManager::GetInstance().RegisterWindowSupportRotationListener(listener);
    EXPECT_EQ(WMError::WM_OK, ret);

    SupportRotationInfo supportRotationInfo;
    supportRotationInfo.displayId_ = 0;
    supportRotationInfo.persistentId_ = 0;
    supportRotationInfo.containerSupportRotation_ = {false, false, false, false};
    supportRotationInfo.sceneSupportRotation_ = {false, false, false, false};
    supportRotationInfo.supportRotationChangeReason_ = "test";
    WindowManager::GetInstance().NotifySupportRotationChange(supportRotationInfo);
    EXPECT_EQ(supportRotationInfo.displayId_, listener->listenerSupportRotationInfo.displayId_);
    EXPECT_EQ(supportRotationInfo.persistentId_, listener->listenerSupportRotationInfo.persistentId_);
    EXPECT_EQ(supportRotationInfo.supportRotationChangeReason_,
        listener->listenerSupportRotationInfo.supportRotationChangeReason_);
}
}
} // namespace
} // namespace Rosen
} // namespace OHOS
